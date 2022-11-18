//
// Created by Max on 11/17/22.
//

#include "pgdata_table.h"

PGDataTable::PGDataTable(std::string mpi_name, std::vector <std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names) {
}

void PGDataTable::add_row(std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows: row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}

void PGDataTable::add_table(PGDataTable table) {
  if(col_names[0] != "Collective") {
    col_names.insert(col_names.begin(), "Collective");
    col_widths.insert(col_widths.begin(), 20);
  }

  int nb_rows = table.get_col_size();
  for (int i = 0; i < nb_rows; i++) {
    for (auto iter = col_names.rbegin(); iter != (col_names.rend()-1); ++iter) {
      std::vector <std::string> values = table.get_values_for_col_name(*iter);
      col_value_map[*iter].push_back(values[i]);
    }
    col_value_map["Collective"].push_back(table.get_mpi_name());
  }
}

void PGDataTable::set_col_widths(std::vector<int> widths) {
  col_widths = widths;
}

void PGDataTable::set_col_names(std::vector <std::string> names) {
  col_names = names;
}

std::string PGDataTable::get_mpi_name() {
  return mpi_name;
}

std::vector <std::string> PGDataTable::get_col_names() {
  return col_names;
}

std::vector<int> PGDataTable::get_col_widths() {
  return col_widths;
}

int PGDataTable::get_col_width(int index) {
  return col_widths.at(index);
}

std::unordered_map <std::string, std::vector<std::string>> PGDataTable::get_col_value_map() {
  return col_value_map;
}

int PGDataTable::get_col_size() {
  return col_value_map.at(col_names[0]).size();
}

std::vector <std::string> PGDataTable::get_values_for_col_name(std::string key) {
  return col_value_map.at(key);
}