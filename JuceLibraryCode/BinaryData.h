/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_90723017_INCLUDED
#define BINARYDATA_H_90723017_INCLUDED

namespace BinaryData
{
    extern const char*   logo_white_png;
    const int            logo_white_pngSize = 7931;
    extern const char*   RedFlash_png;
    const int            RedFlash_pngSize = 11722;
    
    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 2;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
