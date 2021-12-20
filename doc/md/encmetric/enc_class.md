# Encoding classes

A string encoding class is simply a static class that manages how a string is encoded with that encoding. Some encoding classes provided with StringSuire are `ASCII`, `UTF8`, `UTF16BE` `UTF16LE`, `UTF32BE` `UTF32LE`, `Latin1`/`ISO_8859_1`, `ISO_8859_2`, `KOI8_R`, `KOI8_U`, `KOI8_RU`.

Usually encoding classes should be specified as template arguments of string classes, but you can decide todynamically specify an encoding by using the `WIDEchr` template argument. Remember that if you initialize any string with the `WIDEchr` template argument you should pass a dynamic pointer to `EncMetric<unicode>` class representing your encoding, this can be obtained via the `DynEncoding` template argument. For example a dynamic pointer of `UTF8` encofing can be obtained with

    const EncMetric<unicode> *utf8 = DynEncoding<UTF8>::instance();

