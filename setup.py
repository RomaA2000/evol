from setuptools import setup, Extension
from torch.utils.cpp_extension import BuildExtension, CppExtension

ext = CppExtension(
    name='evol_cpp',
    sources=['evol_cpp.cpp', 'market_strategy/strategy.cpp', 'market_strategy/states.cpp', 'deal_tracker/deal_tracker.cpp'],
    include_dirs=['market_strategy', 'deal_tracker', 'utils'],
    language='c++'
)

setup(name='evol_cpp',
      ext_modules=[ext],
      cmdclass={'build_ext': BuildExtension},
      extra_compile_args={"cxx": ["-std=c++17"]}
)
