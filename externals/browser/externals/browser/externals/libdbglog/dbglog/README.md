# libdbglog: Logging library

Dependencis:
* Boost_THREAD
* Boost_SYSTEM

---

Usage:

Logging using C++ stream:

```c++
LOG(level) << "Some number: " << 10 << ".";
```

Logging using Boost.Format:

```c++
LOG(level)("Some number %d.", 10);
```

## Mask grammar

Following is a [ABNF](https://en.wikipedia.org/wiki/ABNF) grammar for log mask syntax. NB: All string are case sensitive.
```
mask = 1*severity / alias                 ; mask is either list of severity settings or one
                                          ; of predefined aliases
severity = debug / info / warning / error ; custom severity 
debug = "D"                               ; debug severity, no granularity
info = "I" level                          ; info severity
warning = "W" level                       ; warning severity
error = "E" level                         ; error severity
level = "1" / "2" / "3" / "4"             ; severity level, 1-4

alias = "DEFAULT"                         ; alias for "I3W2E2" (library default)
        / "ALL"                           ; alias for "DI1W1E1" (log everything)
        / "NONE"                          ; no logging at all (disables fatal severity as well)
        / "VERBOSE"                       ; alias for "I2W2E2" (more verbose than default)
        / "ND"                            ; alias for "I1W1E1" ("ALL" without debug)
```

If multiple levels for one severity are used (e.g. `I1I4` then lowest level wins (i.e. `I3I4` is an equivalent of `I3`).
