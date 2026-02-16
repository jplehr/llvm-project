//===------- Shared Profiler tests - ProfilerTest -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Shared/Profile.h"
#include "Shared/SourceInfo.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/TimeProfiler.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <cstdlib>

class ProfilerTest : public ::testing::Test {
protected:
  static std::optional<std::string>
  readFileToString(const std::filesystem::path &File) {
    std::ifstream Stream(File);
    if (!Stream.is_open())
      return std::nullopt;

    std::string Contents((std::istreambuf_iterator<char>(Stream)),
                         std::istreambuf_iterator<char>());
    return Contents;
  }

  static std::optional<llvm::json::Value>
  parseTraceJSON(const std::string &TraceContents) {
    auto Parsed = llvm::json::parse(TraceContents);
    if (!Parsed)
      return std::nullopt;
    return std::move(*Parsed);
  }

  void SetUp() override {
    // Clear any existing environment variables
    unsetenv("LIBOMPTARGET_PROFILE");
    unsetenv("LIBOMPTARGET_PROFILE_GRANULARITY");
    
    // Create a temporary directory for test output files
    temp_dir = std::filesystem::temp_directory_path() / "profiler_test";
    std::filesystem::create_directories(temp_dir);
    
    test_profile_file = temp_dir / "test_profile.json";
  }

  void TearDown() override {
    // Clean up temporary files and directory
    if (std::filesystem::exists(temp_dir)) {
      std::filesystem::remove_all(temp_dir);
    }

    if (llvm::timeTraceProfilerEnabled())
      llvm::timeTraceProfilerCleanup();
    
    // Clear environment variables
    unsetenv("LIBOMPTARGET_PROFILE");
    unsetenv("LIBOMPTARGET_PROFILE_GRANULARITY");
  }

  std::filesystem::path temp_dir;
  std::filesystem::path test_profile_file;
};

class ProfilerSingletonTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Clear environment variables for singleton tests
    unsetenv("LIBOMPTARGET_PROFILE");
    unsetenv("LIBOMPTARGET_PROFILE_GRANULARITY");
  }

  void TearDown() override {
    unsetenv("LIBOMPTARGET_PROFILE");
    unsetenv("LIBOMPTARGET_PROFILE_GRANULARITY");
  }
};

// Test that Profiler::get() returns a singleton instance
TEST_F(ProfilerSingletonTest, GetReturnsSingletonInstance) {
  Profiler &profiler1 = Profiler::get();
  Profiler &profiler2 = Profiler::get();
  
  // Both references should point to the same object
  EXPECT_EQ(&profiler1, &profiler2);
}

// Test beginSection with string name and detail
TEST_F(ProfilerTest, BeginSectionWithStringRefNameAndDetail) {
  // Set environment variable to enable profiling
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  // These should not crash or throw
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test beginSection with string name and function detail
TEST_F(ProfilerTest, BeginSectionWithStringRefNameAndFunctionDetail) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  auto detailFunction = []() -> std::string {
    return "Dynamic detail from function";
  };
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", detailFunction));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test multiple nested sections
TEST_F(ProfilerTest, NestedSections) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  EXPECT_NO_THROW(profiler.beginSection("OuterFunction", "Outer detail"));
  EXPECT_NO_THROW(profiler.beginSection("MiddleFunction", "Middle detail"));
  EXPECT_NO_THROW(profiler.beginSection("InnerFunction", "Inner detail"));
  
  // End sections in reverse order (LIFO)
  EXPECT_NO_THROW(profiler.endSection()); // InnerFunction
  EXPECT_NO_THROW(profiler.endSection()); // MiddleFunction
  EXPECT_NO_THROW(profiler.endSection()); // OuterFunction
}

// Test sections with various string types
TEST_F(ProfilerTest, BeginSectionWithVariousStringTypes) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  // Test with different string types
  std::string stdName = "StdStringName";
  std::string stdDetail = "StdStringDetail";
  const char* cstrName = "CStringName";
  const char* cstrDetail = "CStringDetail";
  llvm::StringRef llvmName("LLVMStringRefName");
  llvm::StringRef llvmDetail("LLVMStringRefDetail");
  
  EXPECT_NO_THROW(profiler.beginSection(stdName, stdDetail));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection(cstrName, cstrDetail));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection(llvmName, llvmDetail));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with empty strings
TEST_F(ProfilerTest, BeginSectionWithEmptyStrings) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  EXPECT_NO_THROW(profiler.beginSection("", ""));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection("NonEmptyName", ""));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection("", "NonEmptyDetail"));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with very long strings
TEST_F(ProfilerTest, BeginSectionWithLongStrings) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  std::string longName(1000, 'A');
  std::string longDetail(2000, 'B');
  
  EXPECT_NO_THROW(profiler.beginSection(longName, longDetail));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with special characters
TEST_F(ProfilerTest, BeginSectionWithSpecialCharacters) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  std::string specialName = "Test::Function<T>()";
  std::string specialDetail = "Detail with 🚀 unicode and \"quotes\" and \n newlines";
  
  EXPECT_NO_THROW(profiler.beginSection(specialName, specialDetail));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test function detail with various return types
TEST_F(ProfilerTest, BeginSectionWithFunctionDetailVariousReturnTypes) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  // Function returning empty string
  auto emptyDetail = []() -> std::string { return ""; };
  
  // Function returning long string
  auto longDetail = []() -> std::string { 
    return std::string(500, 'X');
  };
  
  // Function returning formatted string
  auto formattedDetail = []() -> std::string {
    return "Generated at line " + std::to_string(__LINE__);
  };
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunc1", emptyDetail));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunc2", longDetail));
  EXPECT_NO_THROW(profiler.endSection());
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunc3", formattedDetail));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test profiling when environment variable is not set
TEST_F(ProfilerTest, ProfilingDisabledWhenEnvironmentVariableNotSet) {
  // Don't set LIBOMPTARGET_PROFILE environment variable
  
  Profiler &profiler = Profiler::get();
  ASSERT_FALSE(llvm::timeTraceProfilerEnabled());
  
  // These should still work without crashing, even when profiling is disabled
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());

  EXPECT_FALSE(std::filesystem::exists(test_profile_file));
}

// Test with custom granularity
TEST_F(ProfilerTest, CustomGranularityEnvironmentVariable) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  setenv("LIBOMPTARGET_PROFILE_GRANULARITY", "100", 1);
  
  Profiler &profiler = Profiler::get();
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with zero granularity
TEST_F(ProfilerTest, ZeroGranularityEnvironmentVariable) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  setenv("LIBOMPTARGET_PROFILE_GRANULARITY", "0", 1);
  
  Profiler &profiler = Profiler::get();
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with negative granularity (should be handled gracefully)
TEST_F(ProfilerTest, NegativeGranularityEnvironmentVariable) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  setenv("LIBOMPTARGET_PROFILE_GRANULARITY", "-100", 1);
  
  Profiler &profiler = Profiler::get();
  
  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());
}

// Test with invalid granularity string
TEST_F(ProfilerTest, InvalidGranularityEnvironmentVariable) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  setenv("LIBOMPTARGET_PROFILE_GRANULARITY", "invalid", 1);
  
  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  EXPECT_NO_THROW(profiler.beginSection("TestFunction", "TestDetail"));
  EXPECT_NO_THROW(profiler.endSection());

  if (llvm::Error Err = llvm::timeTraceProfilerWrite(
          test_profile_file.string(), "invalid-granularity-trace")) {
    ADD_FAILURE() << llvm::toString(std::move(Err));
    return;
  }

  EXPECT_TRUE(std::filesystem::exists(test_profile_file));
}

// Test multiple sequential sections (no nesting)
TEST_F(ProfilerTest, SequentialSections) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  for (int i = 0; i < 10; ++i) {
    std::string name = "Function" + std::to_string(i);
    std::string detail = "Detail" + std::to_string(i);
    
    EXPECT_NO_THROW(profiler.beginSection(name, detail));
    EXPECT_NO_THROW(profiler.endSection());
  }
}

// Test TIMESCOPE macro
TEST_F(ProfilerTest, TimescopeMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  // Test the TIMESCOPE macro in a lambda to create a scope
  auto testScope = []() {
    TIMESCOPE();
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Test TIMESCOPE_WITH_DETAILS macro
TEST_F(ProfilerTest, TimescopeWithDetailsMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  // Test the TIMESCOPE_WITH_DETAILS macro
  auto testScope = []() {
    TIMESCOPE_WITH_DETAILS("Custom details for this scope");
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Stress test with many rapid sections
TEST_F(ProfilerTest, StressTestRapidSections) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  const int numSections = 1000;
  for (int i = 0; i < numSections; ++i) {
    std::string name = "RapidFunction" + std::to_string(i);
    EXPECT_NO_THROW(profiler.beginSection(name, "Rapid detail"));
    EXPECT_NO_THROW(profiler.endSection());
  }
}

// Test deeply nested sections
TEST_F(ProfilerTest, DeeplyNestedSections) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  Profiler &profiler = Profiler::get();
  
  const int depth = 50;
  
  // Begin all nested sections
  for (int i = 0; i < depth; ++i) {
    std::string name = "Level" + std::to_string(i);
    std::string detail = "Depth " + std::to_string(i);
    EXPECT_NO_THROW(profiler.beginSection(name, detail));
  }
  
  // End all nested sections in reverse order
  for (int i = 0; i < depth; ++i) {
    EXPECT_NO_THROW(profiler.endSection());
  }
}

// Test thread safety (basic test - creating profiler from multiple contexts)
// Note: This is a basic test as the Profiler uses LLVM's time profiler which
// should handle thread safety internally
TEST_F(ProfilerTest, BasicThreadSafetyTest) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  // Getting the profiler instance from different contexts should be safe
  Profiler &profiler1 = Profiler::get();
  Profiler &profiler2 = Profiler::get();
  
  EXPECT_EQ(&profiler1, &profiler2);
  
  // Basic operations should not crash
  EXPECT_NO_THROW(profiler1.beginSection("Thread1", "Detail1"));
  EXPECT_NO_THROW(profiler2.beginSection("Thread2", "Detail2"));
  EXPECT_NO_THROW(profiler1.endSection());
  EXPECT_NO_THROW(profiler2.endSection());
}

// Test TIMESCOPE_WITH_IDENT macro with valid ident_t
TEST_F(ProfilerTest, TimescopeWithIdentMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  // Create a valid ident_t structure
  const char* source_str = ";test_file.cpp;test_function;123;45;;";
  ident_t test_ident = {0, 0, 0, 0, source_str};
  
  auto testScope = [&test_ident]() {
    TIMESCOPE_WITH_IDENT(&test_ident);
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Test TIMESCOPE_WITH_NAME_AND_IDENT macro
TEST_F(ProfilerTest, TimescopeWithNameAndIdentMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* source_str = ";test_file.cpp;test_function;456;78;;";
  ident_t test_ident = {0, 0, 0, 0, source_str};
  
  auto testScope = [&test_ident]() {
    TIMESCOPE_WITH_NAME_AND_IDENT("CustomName", &test_ident);
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Test TIMESCOPE_WITH_RTM_AND_IDENT macro
TEST_F(ProfilerTest, TimescopeWithRtmAndIdentMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* source_str = ";test_file.cpp;test_function;789;90;;";
  ident_t test_ident = {0, 0, 0, 0, source_str};
  
  auto testScope = [&test_ident]() {
    TIMESCOPE_WITH_RTM_AND_IDENT(" [parallel region]", &test_ident);
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Test TIMESCOPE_WITH_DETAILS_AND_IDENT macro
TEST_F(ProfilerTest, TimescopeWithDetailsAndIdentMacro) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* source_str = ";test_file.cpp;test_function;321;54;;";
  ident_t test_ident = {0, 0, 0, 0, source_str};
  
  auto testScope = [&test_ident]() {
    TIMESCOPE_WITH_DETAILS_AND_IDENT("MemoryTransfer", " [size=1024]", &test_ident);
    // Some work would happen here
  };
  
  EXPECT_NO_THROW(testScope());
}

// Test macros with nullptr ident_t (should handle gracefully)
TEST_F(ProfilerTest, TimescopeMacrosWithNullIdent) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  ident_t* null_ident = nullptr;
  
  auto testScope1 = [&null_ident]() {
    TIMESCOPE_WITH_IDENT(null_ident);
  };
  
  auto testScope2 = [&null_ident]() {
    TIMESCOPE_WITH_NAME_AND_IDENT("TestName", null_ident);
  };
  
  auto testScope3 = [&null_ident]() {
    TIMESCOPE_WITH_RTM_AND_IDENT(" [test region]", null_ident);
  };
  
  auto testScope4 = [&null_ident]() {
    TIMESCOPE_WITH_DETAILS_AND_IDENT("TestRegion", " [detail]", null_ident);
  };
  
  EXPECT_NO_THROW(testScope1());
  EXPECT_NO_THROW(testScope2());
  EXPECT_NO_THROW(testScope3());
  EXPECT_NO_THROW(testScope4());
}

// Test macros with malformed ident_t source string
TEST_F(ProfilerTest, TimescopeMacrosWithMalformedIdent) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* malformed_source = "malformed_source_string";
  ident_t malformed_ident = {0, 0, 0, 0, malformed_source};
  
  auto testScope = [&malformed_ident]() {
    TIMESCOPE_WITH_IDENT(&malformed_ident);
  };
  
  EXPECT_THROW(testScope(), std::invalid_argument);
}

// Test macros with empty source string
TEST_F(ProfilerTest, TimescopeMacrosWithEmptyIdent) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* empty_source = "";
  ident_t empty_ident = {0, 0, 0, 0, empty_source};
  
  auto testScope = [&empty_ident]() {
    TIMESCOPE_WITH_IDENT(&empty_ident);
  };
  
  EXPECT_THROW(testScope(), std::invalid_argument);
}

// Test multiple macro invocations in nested scopes
TEST_F(ProfilerTest, NestedTimescopeMacros) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  
  const char* outer_source = ";outer.cpp;outer_function;100;10;;";
  const char* inner_source = ";inner.cpp;inner_function;200;20;;";
  ident_t outer_ident = {0, 0, 0, 0, outer_source};
  ident_t inner_ident = {0, 0, 0, 0, inner_source};
  
  auto testNestedScopes = [&outer_ident, &inner_ident]() {
    {
      TIMESCOPE_WITH_IDENT(&outer_ident);
      {
        TIMESCOPE_WITH_NAME_AND_IDENT("InnerScope", &inner_ident);
        {
          TIMESCOPE_WITH_DETAILS("Innermost details");
          // Work happens here
        }
      }
    }
  };
  
  EXPECT_NO_THROW(testNestedScopes());
}

// Test SourceInfo creation with different constructor types
TEST_F(ProfilerTest, SourceInfoConstructors) {
  // Test ident_t constructor
  const char* ident_source = ";test_file.cpp;test_func;123;45;;";
  ident_t test_ident = {0, 0, 0, 0, ident_source};
  
  EXPECT_NO_THROW(SourceInfo si(&test_ident));
  
  // Test map_var_info_t constructor (same format as ident_t for testing)
  const char* map_source = "var_name;test_file.cpp;test_func;123;45;;";
  map_var_info_t map_info = const_cast<char*>(map_source);
  
  EXPECT_NO_THROW(SourceInfo si(map_info));
  
  // Test null pointers
  ident_t* null_ident = nullptr;
  map_var_info_t null_map = nullptr;
  
  EXPECT_NO_THROW(SourceInfo si1(null_ident));
  EXPECT_NO_THROW(SourceInfo si2(null_map));
}

// Test SourceInfo getProfileLocation method
TEST_F(ProfilerTest, SourceInfoGetProfileLocation) {
  const char* source_str = ";test_file.cpp;test_function;123;45;;";
  ident_t test_ident = {0, 0, 0, 0, source_str};
  
  SourceInfo si(&test_ident);
  const char* profile_location = si.getProfileLocation();
  
  EXPECT_NE(profile_location, nullptr);
  EXPECT_STREQ(profile_location, source_str);
}

// Test getNameFromMapping function
TEST_F(ProfilerTest, GetNameFromMapping) {
  const char* map_source = "var_name;test_file.cpp;123;45;;";
  map_var_info_t map_info = const_cast<char*>(map_source);
  
  std::string name = getNameFromMapping(map_info);
  EXPECT_EQ(name, "test_file.cpp");
  
  // Test with null
  std::string null_name = getNameFromMapping(nullptr);
  EXPECT_EQ(null_name, "unknown");
  
  // Test with malformed string
  const char* malformed = "no_semicolons";
  map_var_info_t malformed_info = const_cast<char*>(malformed);
  EXPECT_NO_THROW(getNameFromMapping(malformed_info));
}

TEST_F(ProfilerTest, ProfilerWritesTraceFileWhenEnabled) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);

  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  const std::string EventName = "TraceWriteEvent";
  const std::string EventDetail = "trace detail for verification";

  profiler.beginSection(EventName, EventDetail);
  profiler.endSection();

  if (llvm::Error Err = llvm::timeTraceProfilerWrite(
          test_profile_file.string(), "profiler-trace")) {
    ADD_FAILURE() << llvm::toString(std::move(Err));
    return;
  }

  ASSERT_TRUE(std::filesystem::exists(test_profile_file));

  auto ContentsOpt = readFileToString(test_profile_file);
  ASSERT_TRUE(ContentsOpt.has_value());

  auto JsonOpt = parseTraceJSON(*ContentsOpt);
  ASSERT_TRUE(JsonOpt.has_value());

  auto *Object = JsonOpt->getAsObject();
  ASSERT_NE(Object, nullptr);

  auto *TraceEvents = Object->getArray("traceEvents");
  ASSERT_NE(TraceEvents, nullptr);

  bool FoundName = false;
  bool FoundDetail = false;
  for (const auto &EventValue : *TraceEvents) {
    const auto *EventObj = EventValue.getAsObject();
    if (!EventObj)
      continue;
    if (auto *NameValue = EventObj->getString("name"))
      if (*NameValue == EventName)
        FoundName = true;

    if (auto *DetailValue = EventObj->getString("args")) {
      if (DetailValue->contains(EventDetail))
        FoundDetail = true;
    } else if (const auto *ArgsObj = EventObj->getObject("args")) {
      if (auto *DetailStr = ArgsObj->getString("detail"))
        if (DetailStr->contains(EventDetail))
          FoundDetail = true;
    }

    if (FoundName && FoundDetail)
      break;
  }

  EXPECT_TRUE(FoundName);
  EXPECT_TRUE(FoundDetail);
}

TEST_F(ProfilerTest, BeginSectionLambdaDetailExecutes) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);

  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  bool DetailCalled = false;
  const std::string Detail = "lambda-generated-detail";
  profiler.beginSection("LambdaDetailEvent", [&]() -> std::string {
    DetailCalled = true;
    return Detail;
  });
  profiler.endSection();

  EXPECT_TRUE(DetailCalled);

  if (llvm::Error Err = llvm::timeTraceProfilerWrite(
          test_profile_file.string(), "lambda-trace")) {
    ADD_FAILURE() << llvm::toString(std::move(Err));
    return;
  }

  auto ContentsOpt = readFileToString(test_profile_file);
  ASSERT_TRUE(ContentsOpt.has_value());
  EXPECT_NE(ContentsOpt->find(Detail), std::string::npos);
}

TEST_F(ProfilerTest, InvalidGranularityStillProducesTrace) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);
  setenv("LIBOMPTARGET_PROFILE_GRANULARITY", "invalid", 1);

  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  profiler.beginSection("GranularityEvent", "granularity detail");
  profiler.endSection();

  if (llvm::Error Err = llvm::timeTraceProfilerWrite(
          test_profile_file.string(), "granularity-trace")) {
    ADD_FAILURE() << llvm::toString(std::move(Err));
    return;
  }

  EXPECT_TRUE(std::filesystem::exists(test_profile_file));
}

TEST_F(ProfilerTest, TimescopeMacrosEmitSourceLocationDetails) {
  setenv("LIBOMPTARGET_PROFILE", test_profile_file.c_str(), 1);

  const char *IdentSource = ";TraceSource.cpp;MacroFunction;42;7;;";
  ident_t Ident = {0, 0, 0, 0, IdentSource};

  auto ScopedWork = [&Ident]() { TIMESCOPE_WITH_IDENT(&Ident); };

  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  ScopedWork();
  profiler.beginSection("ManualSection", "manual detail");
  profiler.endSection();

  if (llvm::Error Err = llvm::timeTraceProfilerWrite(
          test_profile_file.string(), "macro-trace")) {
    ADD_FAILURE() << llvm::toString(std::move(Err));
    return;
  }

  auto ContentsOpt = readFileToString(test_profile_file);
  ASSERT_TRUE(ContentsOpt.has_value());
  EXPECT_NE(ContentsOpt->find("TraceSource.cpp"), std::string::npos);
  EXPECT_NE(ContentsOpt->find("MacroFunction"), std::string::npos);
}

TEST_F(ProfilerTest, ProfilerWriteFailurePropagatesError) {
  const std::filesystem::path MissingDir = temp_dir / "unwritable";
  std::filesystem::remove_all(MissingDir);

  const std::filesystem::path TracePath =
      MissingDir / "nested" / "profiler.json";
  setenv("LIBOMPTARGET_PROFILE", TracePath.c_str(), 1);

  Profiler &profiler = Profiler::get();
  ASSERT_TRUE(llvm::timeTraceProfilerEnabled());

  profiler.beginSection("FailureEvent", "detail");
  profiler.endSection();

  llvm::Error Err =
      llvm::timeTraceProfilerWrite(TracePath.string(), "missing-dir");
  EXPECT_TRUE(static_cast<bool>(Err));
  if (Err)
    llvm::consumeError(std::move(Err));

  EXPECT_FALSE(std::filesystem::exists(TracePath));
}
