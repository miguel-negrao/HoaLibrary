/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_4119403_INCLUDED
#define BINARYDATA_H_4119403_INCLUDED

namespace BinaryData
{
    extern const char*   hoa_icon_02_transp_png;
    const int            hoa_icon_02_transp_pngSize = 47111;

    extern const char*   icongear256_png;
    const int            icongear256_pngSize = 7734;

    extern const char*   iconinfoblack256_png;
    const int            iconinfoblack256_pngSize = 7600;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 3;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif