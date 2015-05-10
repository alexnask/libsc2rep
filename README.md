libsc2rep
---------

libsc2rep is a modern (portable) C++ library that aims to provide a high-level, high-performance interface to access Starcraft II replay information.  
I have not found such a library in C or C++ and even if it exists, I highly doubt it is maintained.  
The library will support exclusively Legacy of the Void replays.  
It currently relies on [libmpq](http://github.com/ge0rg/libmpq) for the unpacking out of MoPaQ files, although you can provide individual replay files in form of streams instead.  
This library is intended for use when a huge amount of replays is required to be parsed, efficiency/power needs to be optimal or you absolutely need a native and portable implementation.  
If you absolutely don't need it, I highly suggest the [sc2reader](http://github.com/GraylinKim/sc2reader) python library, which is really well maintained and used widely.  

I am mainly writing this as an exercise and to learn C++11/14 best practises.  
When I have reached a satisfactory set of features, I will probably provide a C interface, so as to facilitate writing bindings for other languages.
