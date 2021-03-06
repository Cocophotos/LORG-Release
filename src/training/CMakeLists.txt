set(tb2gram_sources
      tb2gram.cpp
      ../LorgApp.cpp  LorgTrainerApp.cpp
      ../utils/PtbPsTree.cpp 
      Treebank.cpp 
      ../utils/data_parsers/PTBInputParser.cpp 
      ../utils/SymbolTable.cpp 
      ../rules/Production.cpp 
      ../rules/Rule.cpp 
      ../utils/ConfigTable.cpp 
      EMTrainer.cpp
      ../utils/RandomGenerator.cpp 
      ../rules/BRule.cpp 
      ../rules/URule.cpp
      ../rules/BRuleTraining.cpp 
      ../rules/URuleTraining.cpp
      ../rules/LexicalRule.cpp
      ../rules/LexicalRuleTraining.cpp
      ../grammars/AnnotatedLabelsInfo.cpp
      ../grammars/Grammar.cpp
      ../lexicon/Lexicon.cpp
      ../lexicon/BasicLexicon.cpp
      ../lexicon/BerkeleyLexicon.cpp 
      ../lexicon/LexiconFactory.cpp
      ../lexicon/WordSignatureFactory.cpp
      ../lexicon/BerkeleyEnglishUMapping.cpp
      ../lexicon/BaselineFrenchUMapping.cpp
      ../lexicon/EnglishIGMapping.cpp
      ../lexicon/FrenchIGMapping.cpp
      ../lexicon/ItalianIGMapping.cpp
      ../lexicon/ArabicUMapping.cpp
      ../lexicon/ArabicIGMapping.cpp
      ../edges/AnnotationInfo.cpp
      ../edges/BinaryTrainingTree.cpp
      ../edges/TrainingNode.cpp
      TrainingGrammar.cpp)

set(common_include_dir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${TBB_INCLUDE_DIR})
set(common_libs ${RTLIB} ${TBB_LIBS} ${TCMALLOC_LIB})

#message(warning "In training : Include directories are : ${common_include_dir}")
#message(warning "In training : Common libs are : ${common_libs}")

include_directories(${common_include_dir})

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O6 -Wall -Wextra -pipe -ffast-math -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -pg -ffast-math")
    #set(CMAKE_CXX_FLAGS_DEBUG -Wall -Wextra  -O2  -g  -pg -DBENCHMARK)
    #set(CMAKE_CXX_FLAGS_DEBUG -O1  -g  -Wall -Wextra -pg)
endif()


if (WITH_TB2GRAM EQUAL 1)
    add_executable(tb2gram ${tb2gram_sources})
    target_link_libraries(tb2gram ${common_libs} ${Boost_LIBRARIES})
    # install
    install (TARGETS tb2gram DESTINATION bin)
endif()


