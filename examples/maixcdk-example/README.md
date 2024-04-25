maixcdk-example: example component for MaixCDK
=====

## Install

Install from source:
```
pip install .
```

Install from pypi:
```
pip install maixcdk-example -U
```

Set index url for pip temporarily:
```
pip install -i https://pypi.tuna.tsinghua.edu.cn/simple maixcdk-example
```

Or globally set index url for pip:
```
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
```


## Publish

```
pip install -U twine

python setup.py sdist
# python setup.py sdist bdist_wheel

twine upload dist/*
```




