int var;

#define VAR var

#define FUNC(x) x
#define FUNC2(x) var

#define OTHER(x) (x + 1)

int main(){
  var;
  VAR;
  FUNC(var);
  FUNC(VAR);
  FUNC2(0);
  FUNC2(1);
  OTHER(FUNC(var));
  OTHER(FUNC(VAR));
  OTHER(FUNC2(FUNC(0)));
  OTHER(FUNC2(FUNC(1)));
}

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
