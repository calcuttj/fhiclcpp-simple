#ifndef FHICLCPP_SIMPLE_TYPES_COMPOSITE_TYPES_FROM_IMPL_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_COMPOSITE_TYPES_FROM_IMPL_HXX_SEEN

#include "types/Atom.hxx"
#include "types/Base.hxx"
#include "types/ParameterSet.hxx"
#include "types/Sequence.hxx"

#include "string_parsers/exception.hxx"

#include <memory>

void fhicl::Sequence::from(std::string const &str) {
  if (!str.size()) {
    return;
  }
  std::string tstr = str;
  fhicl::string_parsers::trim(tstr);
  if (!fhicl::string_parsers::well_bracket_wrapped(tstr, '[', ']')) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a non-array-like string ("
        << std::quoted(str) << ") as a Sequence.";

  } else {
    tstr = tstr.substr(1, tstr.size() - 2);
  }
  for (std::string const &outer_list_item :
       fhicl::string_parsers::ParseToVect<std::string>(tstr, ",", true, true)) {
    if (fhicl::string_parsers::is_table(outer_list_item)) {
      internal_rep.push_back(std::make_shared<ParameterSet>(outer_list_item));
    } else if (fhicl::string_parsers::is_sequence(outer_list_item)) {
      internal_rep.push_back(std::make_shared<Sequence>(outer_list_item));
    } else {
      internal_rep.push_back(std::make_shared<Atom>(outer_list_item));
    }
  }
}

void fhicl::ParameterSet::from(std::string const &str) {
  if (!str.size()) {
    return;
  }
  std::string tstr = str;
  fhicl::string_parsers::trim(tstr);
  if (!fhicl::string_parsers::well_bracket_wrapped(tstr, '{', '}')) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a non-map-like string ("
        << std::quoted(str) << ") as a ParameterSet.";

  } else {
    tstr = tstr.substr(1, tstr.size() - 2);
  }
  static const std::map<char, char> extra_brackets{
      {'[', ']'}, {'{', '}'}, {'(', ')'}};
  std::vector<std::string> k_v_list =
      fhicl::string_parsers::ParseToVect<std::string>(tstr, " ", false, true,
                                                      extra_brackets);
  if (k_v_list.size() % 2) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a map-like string but expected an even "
           "number of values to be parsed as key: value pairs: "
        << std::quoted(tstr);
  }

  for (size_t i = 0; i < k_v_list.size(); i += 2) {

    if (k_v_list[i].back() != ':') {
      throw fhicl::string_parsers::parser_fail()
          << "[ERROR]: Attempted to parse a non-key-value-pair-like string ("
          << std::quoted(k_v_list[i] + " " + k_v_list[i + 1])
          << ") as a \"key: value\" pair.";
    }
    std::string const &k = k_v_list[i].substr(0, k_v_list[i].size() - 1);
    std::string const &v = k_v_list[i + 1];

    if (fhicl::string_parsers::is_table(v)) {
      internal_rep.insert({k, std::make_shared<ParameterSet>(v)});
    } else if (fhicl::string_parsers::is_sequence(v)) {
      internal_rep.insert({k, std::make_shared<Sequence>(v)});
    } else {
      internal_rep.insert({k, std::make_shared<Atom>(v)});
    }
  }
}

bool fhicl::ParameterSet::is_key_to_sequence(fhicl::key_t const &key) {
  if (!check_key(key)) {
    return false;
  }
  std::shared_ptr<fhicl::Sequence> seq =
      std::dynamic_pointer_cast<fhicl::Sequence>(internal_rep[key]);
  return bool(seq);
}

template <typename T>
typename std::enable_if<!std::is_same<T, fhicl::ParameterSet>::value &&
                            fhicl::is_seq<T>::value,
                        void>::type
fhicl::ParameterSet::put(key_t const &key, T const &value) {
  if (has_key(key)) {
    throw cant_insert() << "[ERROR]: Cannot put with key: " << std::quoted(key)
                        << " as that key already exists.";
  }
  internal_rep[key] =
      std::make_shared<Sequence>(string_parsers::T2Str<T>(value));
}

#endif
