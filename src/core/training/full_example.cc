//
// Created by Arseny Tolmachev on 2017/10/10.
//

#include "full_example.h"

namespace jumanpp {
namespace core {
namespace training {

Status FullExampleReader::readFullExample(FullyAnnotatedExample *result) {
  result->data_.clear();
  result->lengths_.clear();

  switch (mode_) {
    case DataReaderMode::SimpleCsv:
      return readFullExampleCsv(result);
    case DataReaderMode::DoubleCsv:
      return readFullExampleDblCsv(result);
  }
  return JPPS_NOT_IMPLEMENTED << "example type " << (int)mode_
                              << " is not implemented";
}

Status FullExampleReader::initCsv(StringPiece data) {
  JPP_DCHECK(tio_ != nullptr);
  if (tio_->fields().empty()) {
    return Status::InvalidState()
           << "field data is not initialized, you must do that first";
  }

  mode_ = DataReaderMode::SimpleCsv;
  csv_ = util::CsvReader();
  finished_ = false;
  return csv_.initFromMemory(data);
}

Status FullExampleReader::initDoubleCsv(StringPiece data, char tokenSep,
                                        char fieldSep) {
  JPP_DCHECK(tio_ != nullptr);
  if (tio_->fields().empty()) {
    return Status::InvalidState()
           << "field data is not initialized, you must do that first";
  }

  mode_ = DataReaderMode::DoubleCsv;
  doubleFldSep_ = fieldSep;
  csv_ = util::CsvReader{tokenSep};
  csv2_ = util::CsvReader{fieldSep};
  finished_ = false;
  return csv_.initFromMemory(data);
}

bool startsWith(StringPiece s1, StringPiece s2) {
  if (s2.size() > s1.size()) return false;
  return s1.take(s2.size()) == s2;
}

Status FullExampleReader::readFullExampleCsv(FullyAnnotatedExample *result) {
  while (csv_.nextLine()) {
    if (csv_.numFields() == 1 &&
        (csv_.field(0).empty() || csv_.field(0) == "EOS")) {
      return Status::Ok();
    }

    auto line = csv_.line();
    if (startsWith(line, "# ")) {
      line.slice(0, line.size() - 1).assignTo(result->comment_);
      continue;
    }

    JPP_RETURN_IF_ERROR(readSingleExampleFragment(csv_, result));
  }
  finished_ = true;
  return Status::Ok();
}

Status FullExampleReader::readFullExampleDblCsv(FullyAnnotatedExample *result) {
  finished_ = !csv_.nextLine();
  if (finished_) {
    return Status::Ok();
  }

  auto numwords = csv_.numFields();
  result->data_.reserve(numwords * tio_->fields().size());
  result->lengths_.reserve(numwords);
  for (int i = 0; i < numwords; ++i) {
    auto content = csv_.field(i);

    if (content == "#") {
      for (int j = i + 1; j < numwords; ++j) {
        auto commentPart = csv_.field(j);
        result->comment_.append(commentPart.char_begin(),
                                commentPart.char_end());
        if (j != numwords - 1) {
          result->comment_.push_back(csv_.separator());
        }
      }
      break;
    }

    JPP_RETURN_IF_ERROR(csv2_.initFromMemory(content));
    if (!csv2_.nextLine()) {
      return JPPS_INVALID_PARAMETER << "failed to read word #" << i
                                    << " from the line #" << csv_.lineNumber();
    }
    JPP_RIE_MSG(readSingleExampleFragment(csv2_, result), "data=" << content);
  }

  return Status::Ok();
}

Status FullExampleReader::readSingleExampleFragment(
    const util::CsvReader &csv, FullyAnnotatedExample *result) {
  codepts_.clear();
  auto surfFld = csv.field(tio_->surfaceFieldIdx());

  JPP_RETURN_IF_ERROR(chars::preprocessRawData(surfFld, &codepts_));
  result->lengths_.push_back(codepts_.size());

  auto fields = tio_->fields();

  if (csv.numFields() < fields.size()) {
    return JPPS_INVALID_PARAMETER
           << "a word from the line #" << csv_.lineNumber() << " had "
           << csv.numFields() << " fields, expected " << fields.size();
  }
  result->surface_.append(surfFld.char_begin(), surfFld.char_end());
  for (int i = 0; i < fields.size(); ++i) {
    auto &fldInfo = fields[i];
    auto &map = *fldInfo.str2int;
    auto fld = csv.field(fldInfo.exampleFieldIdx);
    auto it = map.find(fld);
    if (it == map.end()) {
      if (csv_.rowHadQuoted()) {
        if (!charBuffer_.import(&fld)) {
          return JPPS_INVALID_PARAMETER << "failed to import string at file "
                                        << filename_ << ":" << csv_.lineNumber()
                                        << " data=" << fld;
        }
      }
      i32 item = static_cast<i32>(result->strings_.size());
      result->strings_.emplace_back(fld);
      result->data_.push_back(~item);
    } else {
      result->data_.push_back(it->second);
    }
  }
  return Status::Ok();
}

}  // namespace training
}  // namespace core
}  // namespace jumanpp