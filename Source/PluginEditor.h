/*
  ==============================================================================

    PluginEditor.cpp
    PlayMe / Martin Di Rollo - 2014
    GUI of the audio plugin - no actual data processing is done here

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include <fstream>



//==============================================================================
/** This is the editor component that the plugin will display
 */
class SignalProcessorAudioProcessorEditor  : public AudioProcessorEditor,
                                             public SliderListener,
                                             public ComboBoxListener,
                                             public ButtonListener,
                                             public TextEditor::Listener,
                                             public Timer
{
public:
    SignalProcessorAudioProcessorEditor(SignalProcessorAudioProcessor&);

    ~SignalProcessorAudioProcessorEditor();
    
    //==============================================================================
    void timerCallback() override;
    void paint (Graphics&) override;
    //void resized() override;
    void sliderValueChanged (Slider*) override;
    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox*) override;
    void textEditorReturnKeyPressed (TextEditor& editor) override;
    void textEditorTextChanged (TextEditor& editor) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;
    void textEditorFocusLost (TextEditor& editor) override;
    
private:
    SignalProcessorAudioProcessor& audioProcessorInstance;
    
    Label infoLabel, averagingBufferLabel, inputSensitivityLabel, channelLabel, javaAppPathLabel;
    Slider averagingBufferSlider, inputSensitivitySlider;
    ToggleButton sendTimeInfoButton, sendSignalLevelButton, sendImpulseButton, monoStereoButton;
    ImageButton logoButton;
    ComboBox channelComboBox;
    ScopedPointer<ResizableCornerComponent> resizer;
    ComponentBoundsConstrainer resizeLimits;
    
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;
    
    SignalProcessorAudioProcessor& getProcessor() const
    {
        return static_cast<SignalProcessorAudioProcessor&> (processor);
    }
    
    void displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos);
    void setupSquareLookAndFeelColours (LookAndFeel& laf);
    void setAllLookAndFeels (LookAndFeel* laf);
    
    Font pluginFont;
    Font smallFont;
    Image logoImage;

    
    //==============================================================================
    /** Custom Look And Feel subclasss.

        Simply override the methods you need to, anything else will be inherited from the base class.
        It's a good idea not to hard code your colours, use the findColour method along with appropriate
        ColourIds so you can set these on a per-component basis.
     */
    struct CustomLookAndFeel    : public LookAndFeel_V3
    {
        void drawRoundThumb (Graphics& g, const float x, const float y,
                             const float diameter, const Colour& colour, float outlineThickness)
        {
            const Rectangle<float> a (x, y, diameter, diameter);
            const float halfThickness = outlineThickness * 0.5f;

            Path p;
            p.addEllipse (x + halfThickness, y + halfThickness, diameter - outlineThickness, diameter - outlineThickness);

            const DropShadow ds (Colours::black, 1, Point<int> (0, 0));
            ds.drawForPath (g, p);

            g.setColour (colour);
            g.fillPath (p);

            g.setColour (colour.brighter());
            g.strokePath (p, PathStrokeType (outlineThickness));
        }

        void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                                   bool isMouseOverButton, bool isButtonDown) override
        {
            Colour baseColour (backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

            if (isButtonDown || isMouseOverButton)
                baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

            const bool flatOnLeft   = button.isConnectedOnLeft();
            const bool flatOnRight  = button.isConnectedOnRight();
            const bool flatOnTop    = button.isConnectedOnTop();
            const bool flatOnBottom = button.isConnectedOnBottom();

            const float width  = button.getWidth() - 1.0f;
            const float height = button.getHeight() - 1.0f;

            if (width > 0 && height > 0)
            {
                const float cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
                const float lineThickness = cornerSize * 0.1f;
                const float halfThickness = lineThickness * 0.5f;

                Path outline;
                outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                             cornerSize, cornerSize,
                                             ! (flatOnLeft  || flatOnTop),
                                             ! (flatOnRight || flatOnTop),
                                             ! (flatOnLeft  || flatOnBottom),
                                             ! (flatOnRight || flatOnBottom));

                const Colour outlineColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                                                       : TextButton::textColourOffId));

                g.setColour (baseColour);
                g.fillPath (outline);

                if (! button.getToggleState())
                {
                    g.setColour (outlineColour);
                    g.strokePath (outline, PathStrokeType (lineThickness));
                }
            }
        }

        void drawTickBox (Graphics& g, Component& component,
                          float x, float y, float w, float h,
                          bool ticked,
                          bool isEnabled,
                          bool isMouseOverButton,
                          bool isButtonDown) override
        {
            const float boxSize = w * 0.7f;

            bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());
            const Colour colour (component.findColour (TextButton::buttonColourId).withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                 .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f));

            drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, colour,
                            isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

            if (ticked)
            {
                const Path tick (LookAndFeel_V2::getTickShape (6.0f));
                g.setColour (isEnabled ? findColour (TextButton::buttonOnColourId) : Colours::grey);

                const float scale = 9.0f;
                const AffineTransform trans (AffineTransform::scale (w / scale, h / scale)
                                                 .translated (x - 2.5f, y + 1.0f));
                g.fillPath (tick, trans);
            }
        }

        void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, float minSliderPos, float maxSliderPos,
                                    const Slider::SliderStyle style, Slider& slider) override
        {
            const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

            bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
            Colour knobColour (slider.findColour (Slider::thumbColourId).withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));

            if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
            {
                float kx, ky;

                if (style == Slider::LinearVertical)
                {
                    kx = x + width * 0.5f;
                    ky = sliderPos;
                }
                else
                {
                    kx = sliderPos;
                    ky = y + height * 0.5f;
                }

                const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

                drawRoundThumb (g,
                                kx - sliderRadius,
                                ky - sliderRadius,
                                sliderRadius * 2.0f,
                                knobColour, outlineThickness);
            }
            else
            {
                // Just call the base class for the demo
                LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }

        void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               const Slider::SliderStyle style, Slider& slider) override
        {
            g.fillAll (slider.findColour (Slider::backgroundColourId));

            if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
            {
                const float fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;

                Path p;

                if (style == Slider::LinearBarVertical)
                    p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
                else
                    p.addRectangle (fx, fy, sliderPos - fx, fh);


                Colour baseColour (slider.findColour (Slider::rotarySliderFillColourId)
                                   .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                   .withMultipliedAlpha (0.8f));

                g.setColour (baseColour);
                g.fillPath (p);

                const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
                g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
            }
            else
            {
                drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
                drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }

        void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height,
                                         float /*sliderPos*/,
                                         float /*minSliderPos*/,
                                         float /*maxSliderPos*/,
                                         const Slider::SliderStyle /*style*/, Slider& slider) override
        {
            const float sliderRadius = getSliderThumbRadius (slider) - 5.0f;
            Path on, off;

            if (slider.isHorizontal())
            {
                const float iy = x + width * 0.5f - sliderRadius * 0.5f;
                Rectangle<float> r (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius);
                const float onW = r.getWidth() * ((float) slider.valueToProportionOfLength (slider.getValue()));

                on.addRectangle (r.removeFromLeft (onW));
                off.addRectangle (r);
            }
            else
            {
                const float ix = x + width * 0.5f - sliderRadius * 0.5f;
                Rectangle<float> r (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius);
                const float onH = r.getHeight() * ((float) slider.valueToProportionOfLength (slider.getValue()));

                on.addRectangle (r.removeFromBottom (onH));
                off.addRectangle (r);
            }

            g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
            g.fillPath (on);

            g.setColour (slider.findColour (Slider::trackColourId));
            g.fillPath (off);
        }

        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                               float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
        {
            const float radius = jmin (width / 2, height / 2) - 2.0f;
            const float centreX = x + width * 0.5f;
            const float centreY = y + height * 0.5f;
            const float rx = centreX - radius;
            const float ry = centreY - radius;
            const float rw = radius * 2.0f;
            const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

            if (slider.isEnabled())
                g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
            else
                g.setColour (Colour (0x80808080));

            {
                Path filledArc;
                filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
                g.fillPath (filledArc);
            }

            {
                const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
                Path outlineArc;
                outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
                g.strokePath (outlineArc, PathStrokeType (lineThickness));
            }
        }
    };

    //==============================================================================
    /** Another really simple look and feel that is very flat and square.
        This inherits from CustomLookAndFeel above for the linear bar and slider backgrounds.
     */
    struct SquareLookAndFeel    : public CustomLookAndFeel
    {
        void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                                   bool isMouseOverButton, bool isButtonDown) override
        {
            Colour baseColour (backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

            if (isButtonDown || isMouseOverButton)
                baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

            const float width  = button.getWidth() - 1.0f;
            const float height = button.getHeight() - 1.0f;

            if (width > 0 && height > 0)
            {
                g.setGradientFill (ColourGradient (baseColour, 0.0f, 0.0f,
                                                   baseColour.darker (0.1f), 0.0f, height,
                                                   false));

                g.fillRect (button.getLocalBounds());
            }
        }

        void drawTickBox (Graphics& g, Component& component,
                          float x, float y, float w, float h,
                          bool ticked,
                          bool isEnabled,
                          bool /*isMouseOverButton*/,
                          bool /*isButtonDown*/) override
        {
            const float boxSize = w * 0.7f;

            bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());
            const Colour colour (component.findColour (TextButton::buttonOnColourId).withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                 .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f));
            g.setColour (colour);

            Rectangle<float> r (x, y + (h - boxSize) * 0.5f, boxSize, boxSize);
            g.fillRect (r);

            if (ticked)
            {
                const Path tick (LookAndFeel_V3::getTickShape (6.0f));
                g.setColour (isEnabled ? findColour (TextButton::buttonColourId) : Colours::grey);

                const AffineTransform trans (RectanglePlacement (RectanglePlacement::centred)
                                             .getTransformToFit (tick.getBounds(), r.reduced (r.getHeight() * 0.05f)));
                g.fillPath (tick, trans);
            }
        }

        void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, float minSliderPos, float maxSliderPos,
                                    const Slider::SliderStyle style, Slider& slider) override
        {
            const float sliderRadius = (float) getSliderThumbRadius (slider);

            bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
            Colour knobColour (slider.findColour (Slider::rotarySliderFillColourId).withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));
            g.setColour (knobColour);

            if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
            {
                float kx, ky;

                if (style == Slider::LinearVertical)
                {
                    kx = x + width * 0.5f;
                    ky = sliderPos;
                    g.fillRect (Rectangle<float> (kx - sliderRadius, ky - 2.5f, sliderRadius * 2.0f, 5.0f));
                }
                else
                {
                    kx = sliderPos;
                    ky = y + height * 0.5f;
                    g.fillRect (Rectangle<float> (kx - 2.5f, ky - sliderRadius, 5.0f, sliderRadius * 2.0f));
                }
            }
            else
            {
                // Just call the base class for the demo
                LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }

        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                               float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
        {
            const float diameter = jmin (width, height) - 4.0f;
            const float radius = (diameter / 2.0f) * std::cos (float_Pi / 4.0f);
            const float centreX = x + width * 0.5f;
            const float centreY = y + height * 0.5f;
            const float rx = centreX - radius;
            const float ry = centreY - radius;
            const float rw = radius * 2.0f;
            const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

            const Colour baseColour (slider.isEnabled() ? slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 0.8f : 1.0f)
                                                        : Colour (0x80808080));

            Rectangle<float> r (rx, ry, rw, rw);
            AffineTransform t (AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY()));

            float x1 = r.getTopLeft().getX(), y1 = r.getTopLeft().getY(), x2 = r.getBottomLeft().getX(), y2 = r.getBottomLeft().getY();
            t.transformPoints (x1, y1, x2, y2);

            g.setGradientFill (ColourGradient (baseColour, x1, y1,
                                               baseColour.darker (0.1f), x2, y2,
                                               false));

            Path knob;
            knob.addRectangle (r);
            g.fillPath (knob, t);

            Path needle;
            Rectangle<float> r2 (r * 0.1f);
            needle.addRectangle (r2.withPosition (Point<float> (r.getCentreX() - (r2.getWidth() / 2.0f), r.getY())));

            g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
            g.fillPath (needle, AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY()));
        }
    };

    SquareLookAndFeel* slaf;



};


#endif  // PLUGINEDITOR_H_INCLUDED
