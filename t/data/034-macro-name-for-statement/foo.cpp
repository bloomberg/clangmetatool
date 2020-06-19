int var;

#define VAR var

#define FUNC(x) (x + 1)

extern void vararg_func(int x, ...);
#define VARARG_FUNC(...) vararg_func(0, ##__VA_ARGS__)

int main(){
  var;
  VAR;
  FUNC(var);
  FUNC(VAR);
  VARARG_FUNC(var);
  VARARG_FUNC(VAR);
}

// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
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
