
// Copyright 2018, Bosch Software Innovations GmbH.
// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "rosbag2_cpp/reindexer.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "rosbag2_cpp/info.hpp"
#include "rosbag2_cpp/reindexer_interfaces/base_reindexer_interface.hpp"

namespace rosbag2_cpp
{

Reader::Reader(std::unique_ptr<reader_interfaces::BaseReaderInterface> reader_impl)
: reader_impl_(std::move(reader_impl))
{}

Reader::~Reader()
{
  reader_impl_->reset();
}

void Reader::open(
  const StorageOptions & storage_options,
  const ConverterOptions & converter_options)
{
  reader_impl_->open(storage_options, converter_options);
}

bool Reader::has_next()
{
  return reader_impl_->has_next();
}

} //namespace rosbag2_cpp