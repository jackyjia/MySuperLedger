/************************************************************************
Copyright 2019-2021 eBay Inc.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    https://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#ifndef SRC_INFRA_UTIL_HTTPCODE_H_
#define SRC_INFRA_UTIL_HTTPCODE_H_
namespace gringofts {

class HttpCode {
 public:
  static constexpr int OK = 200;
  static constexpr int CREATED = 201;

  static constexpr int MOVED_PERMANENTLY = 301;

  static constexpr int BAD_REQUEST = 400;
  static constexpr int FORBIDDEN = 403;
  static constexpr int CONFLICT = 409;

  static constexpr int SERVICE_UNAVAILABLE = 503;
  static constexpr int INTERNAL_SERVER_ERROR = 500;
};


}  // namespace gringofts
#endif  // SRC_INFRA_UTIL_HTTPCODE_H_
