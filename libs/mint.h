#ifndef MINT_H
#define MINT_H

/*
This file contains common things for each header only library in libmint.
If you copy one of the header libs you should copy this file as well and compile it with your project.
*/


// Change this to change how Loggo is compiled in
// Static would obviously make scope Loggo API methods to the 
// current translation unit which could be what you want
// Defaults to extern API.
#ifndef MINT_DEF
    #ifdef MINT_DEF_STATIC
        #define MINT_DEF static
    #else
        #define MINT_DEF extern
    #endif
#endif



#ifdef MINT_IMPLEMENTATION


#endif // MINT_IMPLEMENTATION
#undef MINT_IMPLEMENTATION

#endif // MINT_H
