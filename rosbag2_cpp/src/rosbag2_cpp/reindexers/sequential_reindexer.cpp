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

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "rcpputils/asserts.hpp"
#include "rcpputils/filesystem_helper.hpp"

#include "rosbag2_cpp/logging.hpp"
#include "rosbag2_cpp/reindexers/sequential_reindexer.hpp"


namespace rosbag2_cpp
{
namespace reindexers
{
namespace details
{
std::vector<std::string> resolve_relative_paths(
  const std::string & base_folder, std::vector<std::string> relative_files, const int version = 4)
{
  auto base_path = rcpputils::fs::path(base_folder);
  if (version < 4) {
    // In older rosbags (version <=3) relative files are prefixed with the rosbag folder name
    base_path = rcpputils::fs::path(base_folder).parent_path();
  }

  rcpputils::require_true(
    base_path.exists(), "base folder does not exist: " + base_folder);
  rcpputils::require_true(
    base_path.is_directory(), "base folder has to be a directory: " + base_folder);

  for (auto & file : relative_files) {
    auto path = rcpputils::fs::path(file);
    if (path.is_absolute()) {
      continue;
    }
    file = (base_path / path).string();
  }

  return relative_files;
}
}  // namespace details

SequentialReindexer::SequentialReindexer(
  std::unique_ptr<rosbag2_storage::StorageFactoryInterface> storage_factory,
  std::shared_ptr<SerializationFormatConverterFactoryInterface> converter_factory,
  std::unique_ptr<rosbag2_storage::MetadataIo> metadata_io)
: storage_factory_(std::move(storage_factory)),
  converter_(nullptr),
  metadata_io_(std::move(metadata_io)),
  converter_factory_(std::move(converter_factory))
{}

SequentialReindexer::~SequentialReindexer()
{
  reset();
}

void SequentialReindexer::open(
  const StorageOptions & storage_options, const ConverterOptions & converter_options)
{
  // If there is a metadata.yaml file present, load it.
  // If not, let's ask the storage with the given URI for its metadata.
  // This is necessary for non ROS2 bags (aka ROS1 legacy bags).
  if (metadata_io_->metadata_file_exists(storage_options.uri)) {
    metadata_ = metadata_io_->read_metadata(storage_options.uri);
    if (metadata_.relative_file_paths.empty()) {
      ROSBAG2_CPP_LOG_WARN("No file paths were found in metadata.");
      return;
    }

    file_paths_ = details::resolve_relative_paths(
      storage_options.uri, metadata_.relative_file_paths, metadata_.version);
    current_file_iterator_ = file_paths_.begin();

    storage_ = storage_factory_->open_read_only(
      get_current_file(), storage_options.storage_id);
    if (!storage_) {
      throw std::runtime_error{"No storage could be initialized. Abort"};
    }
  } else {
    storage_ = storage_factory_->open_read_only(
      storage_options.uri, storage_options.storage_id);
    if (!storage_) {
      throw std::runtime_error{"No storage could be initialized. Abort"};
    }
    metadata_ = storage_->get_metadata();
    if (metadata_.relative_file_paths.empty()) {
      ROSBAG2_CPP_LOG_WARN("No file paths were found in metadata.");
      return;
    }
    file_paths_ = metadata_.relative_file_paths;
    current_file_iterator_ = file_paths_.begin();
  }
  auto topics = metadata_.topics_with_message_count;
  if (topics.empty()) {
    ROSBAG2_CPP_LOG_WARN("No topics were listed in metadata.");
    return;
  }
  fill_topics_metadata();

  // Currently a bag file can only be played if all topics have the same serialization format.
  check_topics_serialization_formats(topics);
  check_converter_serialization_format(
    converter_options.output_serialization_format,
    topics[0].topic_metadata.serialization_format);
}

void SequentialReindexer::fill_topics_metadata()
{
  rcpputils::check_true(storage_ != nullptr, "Bag is not open. Call open() before reading.");
  topics_metadata_.clear();
  topics_metadata_.reserve(metadata_.topics_with_message_count.size());
  for (const auto & topic_information : metadata_.topics_with_message_count) {
    topics_metadata_.push_back(topic_information.topic_metadata);
  }
}

void SequentialReindexer::init_metadata()
{
  metadata_ = rosbag2_storage::BagMetadata{};
  metadata_.storage_identifier = storage_->get_storage_identifier();
  metadata_.starting_time = std::chrono::time_point<std::chrono::high_resolution_clock>(
    std::chrono::nanoseconds::max());
  metadata_.relative_file_paths = {strip_parent_path(storage_->get_relative_file_path())};
}

void SequentailReindexer::reindex()
{
  // init_metadata();  // Create a baseline to start from
  throw std::runtime_error("Successfully called Reindex!")


}

}  // namespace readers
}  // namespace rosbag2_cpp
