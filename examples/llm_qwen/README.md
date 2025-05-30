LLM Qwen Project based on MaixCDK
====

Run LLM Qwen eample.

build method please visit [MaixCDK](https://github.com/sipeed/MaixCDK).

| Platform | Support |
| -------- | ------- |
| MaixCAM2 | ✅ |
| MaixCAM  | ❌ |

**Attention**: Run LLM will cost a lot of memory, please make sure you have enough memory to run it.
For example, on MaixCAM2, Qwen2.5-1.5B need 2GB memory, Qwen2.5-0.5B need 1GB memory.
we can use python scripts to check memory info:

```python
from maix import sys

print(sys.memory_info())
```

Then output like this:

```
{'cma_total': 0, 'cma_used': 0, 'cmm_total': 2147483648, 'cmm_used': 177512448, 'hw_total': 4294967296, 'total': 2060726272, 'used': 338223104}
```

NPU use `cmm_total` memory, in MaixCAM2 also called `CMM` memory, not use `total` memory, so you should ensure `cmm_total` is enough to run LLM model.
Here we can see `cmm_total` is `2GB`, so we can run Qwen2.5-1.5B model.
More memory intro please visit [MaixPy document)(https://wiki.sipeed.com/maixpy/en/pro/memory.html).







