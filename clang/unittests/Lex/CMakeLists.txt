add_clang_unittest(LexTests
  DependencyDirectivesScannerTest.cpp
  HeaderMapTest.cpp
  HeaderSearchTest.cpp
  LexerTest.cpp
  LexHLSLRootSignatureTest.cpp
  ModuleDeclStateTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
  PPDependencyDirectivesTest.cpp
  PPMemoryAllocationsTest.cpp
  CLANG_LIBS
  clangAST
  clangBasic
  clangLex
  clangParse
  clangSema
  LINK_LIBS
  LLVMTestingAnnotations
  LLVMTestingSupport
  LLVM_COMPONENTS
  Support
  )
