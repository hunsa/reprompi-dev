/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/
#include "pgdata_table.h"

PGDataTable::PGDataTable(const std::string& title, std::vector <std::string> column_names) :
    title(title), column_names(column_names) {
}

int PGDataTable::get_col_width(int index) {
  return col_widths.at(index);
}

int PGDataTable::get_col_size() {
  return col_value_map.at(column_names[0]).size();
}

std::string PGDataTable::get_title() {
  return title;
}

std::vector<int> PGDataTable::get_col_widths() {
  return col_widths;
}

std::vector <std::string> PGDataTable::get_column_names() {
  return column_names;
}

std::vector <std::string> PGDataTable::get_values_for_col_name(std::string key) {
  return col_value_map.at(key);
}

std::string PGDataTable::get_values_col_row(std::string col, int row) {
  return col_value_map.at(col).at(row);
}

std::unordered_map <std::string, std::vector<std::string>> PGDataTable::get_col_value_map() {
  return col_value_map;
}

void PGDataTable::set_col_widths(const std::vector<int>& widths) {
  col_widths = widths;
}

void PGDataTable::set_column_names(const std::vector <std::string>& names) {
  column_names = names;
}

void PGDataTable::add_row(const std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows : row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}

void PGDataTable::add_table(PGDataTable table) {
  if (column_names[0] != "collective") {
    column_names.insert(column_names.begin(), "collective");
    col_widths.insert(col_widths.begin(), 20);
  }

  int nb_rows = table.get_col_size();
  for (int i = 0; i < nb_rows; i++) {
    for (auto iter = column_names.rbegin(); iter != (column_names.rend() - 1); ++iter) {
      col_value_map[*iter].push_back(table.get_values_col_row(*iter, i));
    }
    col_value_map["collective"].push_back(table.get_title());
  }
}

PGDataTable PGDataTable::get_violation_table() {
  std::vector<int> col_widths = {25, 15, 15, 20, 10};
  PGDataTable res("stats", {"collective", "brave_sum", "cautious_sum", "barrier_warnings", "of"});
  std::vector <std::string> violations;
  std::vector <std::string> barriers;
  std::string mode = "";

  for (const std::string &col_name : column_names) {
    if (col_name == "diff<barrier") {
      violations = get_values_for_col_name("violation");
      barriers = get_values_for_col_name("diff<barrier");
      mode = "violation";
      break;
    }
    if (col_name == "violation") {
      violations = get_values_for_col_name("violation");
      barriers = get_values_for_col_name("mockup");
      mode = "violation";
    }
    if (col_name == "fastest_mockup") {
      violations = get_values_for_col_name("fastest_mockup");
      barriers = get_values_for_col_name("fastest_mockup");
      mode = "fastest";
    }
  }

  size_t row_idx = 0;
  int brave_coll = 0;
  int cautious_coll = 0;
  int brave_sum = 0;
  int cautious_sum = 0;
  int of_coll = 0;
  int of_sum = 0;
  std::string coll_name;

  for (std::string violation : violations) {
    if (mode != "violation" || get_values_col_row("mockup", row_idx) != "default") {
      of_coll++;
    }

    coll_name = get_values_col_row("collective", row_idx);

    if (mode == "violation") {
      if (violation != "" && violation == "1") {
        brave_coll++;
        if (barriers.at(row_idx).back() != '*') {
          cautious_coll++;
        }
      }
    } else {
      if (violation != "") {
        brave_coll++;
        if (barriers.at(row_idx).back() != '*') {
          cautious_coll++;
        }
      }
    }

    if (row_idx == (violations.size() - 1) || coll_name != get_values_col_row("collective", ++row_idx)) {
      std::unordered_map <std::string, std::string> row;
      row["collective"] = coll_name;
      row["brave_sum"] = std::to_string(brave_coll);
      row["cautious_sum"] = std::to_string(cautious_coll);
      row["barrier_warnings"] = std::to_string(brave_coll - cautious_coll);
      row["of"] = std::to_string(of_coll);
      res.add_row(row);
      brave_sum += brave_coll, cautious_sum += cautious_coll, of_sum += of_coll;
      of_coll = 0, brave_coll = 0, cautious_coll = 0;
    }
  }

  brave_sum += brave_coll;
  cautious_sum += cautious_coll;

  std::unordered_map <std::string, std::string> row;
  row["collective"] = "Sum";
  row["brave_sum"] = std::to_string(brave_sum);
  row["cautious_sum"] = std::to_string(cautious_sum);
  row["barrier_warnings"] = std::to_string(brave_sum - cautious_sum);
  row["of"] = std::to_string(of_sum);
  res.add_row(row);
  res.set_col_widths(col_widths);
  return res;
}

std::string PGDataTable::to_string() {
  const int nb_rows = get_col_size();

  std::ostringstream res;
  res.exceptions(std::ios::failbit);
  res.rdbuf()->pubsetbuf(nullptr, 0);
  res.precision(10);
  res << std::fixed;
  res.exceptions(std::ios::goodbit);
  res.str().reserve(column_names.size() * 10 + nb_rows * column_names.size() * 20);

  if (!title.empty()) {
    res << "MPI Collective: " << title << "\n";
  } else {
    res << "MPI All Results " << "\n";
  }

  int idx = 0;
  for (auto col_name = column_names.cbegin(); col_name != column_names.cend(); ++col_name) {
    res << std::left << std::setw(get_col_width(idx++)) << *col_name << " ";
  }
  res << "\n";

  for (int i = 0; i != nb_rows; ++i) {
    idx = 0;
    for (auto col_name = column_names.cbegin(); col_name != column_names.cend(); ++col_name) {
      res << std::left << std::setw(get_col_width(idx++)) << get_values_col_row(*col_name, i) << " ";
    }
    res << "\n";
  }
  return res.str();
}

std::string PGDataTable::to_csv_string() {
  const std::string col_delimiter = ",";
  const std::string row_delimiter = "\n";

  const int nb_rows = get_col_size();

  std::ostringstream res;
  res.exceptions(std::ios::failbit);
  res.rdbuf()->pubsetbuf(nullptr, 0);
  res.precision(10);
  res << std::fixed;
  res.exceptions(std::ios::goodbit);
  res.str().reserve(column_names.size() * 10 + nb_rows * column_names.size() * 20);

  for (auto col_name = column_names.cbegin(); col_name != column_names.cend(); ++col_name) {
    res << *col_name;
    if (std::next(col_name) != column_names.cend()) {
      res << col_delimiter;
    }
  }
  res << row_delimiter;

  for (int i = 0; i != nb_rows; ++i) {
    for (auto col_name = column_names.cbegin(); col_name != column_names.cend(); ++col_name) {
      res << get_values_col_row(*col_name, i);
      if (std::next(col_name) != column_names.cend()) {
        res << col_delimiter;
      }
    }
    if (i != nb_rows - 1) {
      res << row_delimiter;
    }
  }
  return res.str();
}
