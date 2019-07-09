
# Server Protocol

+--------+                    +--------+
| Client |                    | Server |
+--------+                    +--------+
    |                             |
    |----- HTPMessageHeader ----->|
    |----- HTPMessage ----------->|
    |                             |
    |<------HTPMessageHeader------|
    |<------HTPMessage -----------|
    |
    |
    V