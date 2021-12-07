int begin;
int end;

#define MACRO begin + end
#define BEGIN begin
#define END end
#define WITH_PARENS ( ( begin + end ) )
#define TRANSITIVE_MACRO BEGIN + END
#define MATCH_IN_BODY(x) begin + end
#define TRANSITIVE_MATCH_IN_BODY(y) MATCH_IN_BODY(y)

#define FUNC(x) x
#define PARTIAL_FUNC(x) 1 && x && 1

#define PARTIAL begin + end && 1
#define PARTIAL2 1 && begin + end
#define PARTIAL3 1 && WITH_PARENS && 1
#define PARTIAL4 1 && FUNC(begin + end) && 1
#define PARTIAL5 1 && FUNC(FUNC(begin + end)) && 1
#define PARTIAL6 1 && FUNC(FUNC(begin + end) && 1) && 1
#define PARTIAL7 1 && FUNC(1 && FUNC(begin + end)) && 1
#define PARTIAL8 1 && MATCH_IN_BODY(4) && 1
#define PARTIAL9 1&& TRANSITIVE_MACRO && 1
#define PARTIAL10 1 && FUNC(TRANSITIVE_MACRO) && 1

#define ADD_BEGIN(x) begin + x
#define ADD_END(x) x + end
#define TRANSITIVE_INVALID_MACRO ADD_BEGIN(end)
#define END_WITH_EXTRA end && 1
#define SPLIT_MACROS begin + END_WITH_EXTRA

int main() {
  // These cases can be fully expanded

  begin + end;                 // begin + end
  BEGIN + END;                 // BEGIN + END
  MACRO;                       // MACRO
  WITH_PARENS;                 // WITH_PARENS
  TRANSITIVE_MACRO;            // TRANSITIVE_MACRO
  MATCH_IN_BODY(1);            // MATCH_IN_BODY(1)
  TRANSITIVE_MATCH_IN_BODY(2); // TRANSITIVE_MATCH_IN_BODY(2)

  // These cases expand partially

  FUNC(begin + end);               // begin + end
  FUNC(FUNC(begin + end));         // begin + end
  FUNC(begin + end && 1);          // begin + end
  FUNC(1 && begin + end);          // begin + end
  FUNC(FUNC(begin + end) && 1);    // begin + end
  FUNC(1 && FUNC(begin + end));    // begin + end
  FUNC(MATCH_IN_BODY(3));          // MATCH_IN_BODY(3)
  PARTIAL_FUNC(begin + end);       // begin + end
  PARTIAL_FUNC(MACRO);             // MACRO
  PARTIAL_FUNC(TRANSITIVE_MACRO);  // TRANSITIVE_MACRO

  // These cases expand inside a macro

  PARTIAL;   // begin + end
  PARTIAL2;  // begin + end
  PARTIAL3;  // WITH_PARENS
  PARTIAL4;  // begin + end
  PARTIAL5;  // begin + end
  PARTIAL6;  // begin + end
  PARTIAL7;  // begin + end
  PARTIAL8;  // MATCH_IN_BODY(4)
  PARTIAL9;  // TRANSITIVE_MACRO
  PARTIAL10; // TRANSITIVE_MACRO

  // These cases are invalid (empty comment = invalid)

  ADD_BEGIN(end);           //
  ADD_END(begin);           //
  TRANSITIVE_INVALID_MACRO; //
  SPLIT_MACROS;             //
}

// ----------------------------------------------------------------------------
// Copyright 2021 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
