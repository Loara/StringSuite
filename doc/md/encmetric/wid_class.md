# Wide encodings

Usually encoding classes should be specified as template arguments of string classes, but you can decide todynamically specify an encoding by using the `sts::WIDE<ctype>` template argument (with `sts::WIDEchr = sts::WIDE<sts::unicode>`). Class `sts::WIDE<ctype>` is not an encoding class but is a simple placeholder, saying that all the encoding information should be passed at runtime via an `const sts::EncMetric<ctype> *` argument representing your encoding. In this way you can work with string with unknown encoding at compile time.

Instances of `sts::EncMetric<ctype>` classes should be obtained via the `DynEncoding::instance()` static member function. For example if you want an `sts::EncMetric<sts::unicode>` pointer pointing to an `sts::UTF8` encoding you can use the following expression:

    const sts::EncMetric<sts::unicode> *utf8 = sts::DynEncoding<sts::UTF8>::instance();

You can use the `sts::widenc` concept to determine if a clas is equal to `sts::WIDE`, also the `sts::general_enctype` concept tests if the specified class is an encoding class or a `sts::WIDE` class.

## `Encmetric_info` class

If you want to implement a class that works both with encoding classes and `sts::EncMetric` pointers you can add an `sts::Encmetric_info` member that treats them in the same manner via an unique interface. See the `encmetric.hpp` class for more informations.
