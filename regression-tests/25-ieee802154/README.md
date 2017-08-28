# Regression Tests of IEEE 802.15.4 Fundamentals

## 01-panid-handling

Test return values by `frame802154_has_panid()` in
[frame802154.c](../../core/net/mac/frame802154.c).

### Test Code

A test vector is implemented in
[test-panid-handling.c](./code/test-panid-handling.c) according to Section
7.2.1.5, IEEE 802.15.4-2015. The testee mote outputs a test result to the
console with the prefix, `"=check-me="`.

[01-panid-handling.js](./js/01-panid-handling.js) examines each console output
containing `"=check-me="`. If it finds `"DONE"` without having had any
`"FAILED"`, the test is considered SUCCESS, `log.testOK()` is called. Otherwise,
FAILED.

### References

* https://standards.ieee.org/findstds/standard/802.15.4-2015.html
* https://github.com/contiki-os/contiki/pull/1914