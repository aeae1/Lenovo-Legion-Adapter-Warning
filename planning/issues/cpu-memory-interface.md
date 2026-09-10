# Investigate the older CPU Architectural Protocol as a possible fallback

Research only; V4 must remain unchanged while hardware testing is pending.

The newer Memory Attribute Protocol GUID was absent from the decompressed module scan. CpuDxe does install the older CPU Architectural Protocol, and its SetMemoryAttributes function was identified statically. See docs/research/REVIEW-AND-CPU-PROTOCOL.md for GUIDs, RVAs, source links, and limitations.

Investigate live availability, state-dependent no-op behavior, preservation of caching/read/execute attributes, page granularity, post-operation page-table checks, and restoration. Define targeted failure cases before considering a new version. Do not assume success from a return code or implement CR0.WP bypasses.

Completion: a documented supported or unsupported conclusion with evidence. A V4 PERMISSION_BLOCKED result alone does not settle this research question. No live fallback has been implemented or validated.
