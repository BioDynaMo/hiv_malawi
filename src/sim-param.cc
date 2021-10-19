#include "sim-param.h"

namespace bdm {

void SimParam::SetSociobehavMixingMatrix() {
  sociobehav_mixing_matrix.clear();
  sociobehav_mixing_matrix.resize(nb_age_categories);

  // ToDo (Aziza, AM): should the line below be  `< nb_age_categories`?
  for (int i = 0; i < nb_sociobehav_categories; i++) {
    sociobehav_mixing_matrix[i].resize(nb_sociobehav_categories);
    // Fill all elements with 1.0. Homogeneous socio-behavioural mixing.
    fill(sociobehav_mixing_matrix[i].begin(), sociobehav_mixing_matrix[i].end(),
         1.0);
  }
};

void SimParam::SetAgeMixingMatrix() {
  age_mixing_matrix.clear();
  age_mixing_matrix.resize(nb_age_categories);

  for (int i = 0; i < nb_age_categories; i++) {
    age_mixing_matrix[i].resize(nb_age_categories);
    // Fill all elements with 1.0. Homogeneous age mixing.
    fill(age_mixing_matrix[i].begin(), age_mixing_matrix[i].end(), 1.0);
  }
};

void SimParam::SetLocationMixingMatrix() {
  int nb_locations = Location::kLocLast;
  location_mixing_matrix.clear();
  location_mixing_matrix.resize(nb_locations);

  for (int i = 0; i < nb_locations; i++) {
    location_mixing_matrix[i].resize(nb_locations);
    // Fill all elements with 0.0 except diagonal with 1.0.
    /*fill(location_mixing_matrix[i].begin(),location_mixing_matrix[i].end(),
    0.0); location_mixing_matrix[i][i]=1.0;*/
    // Fill all elements with 1.0 (Homogeneous mixing)
    fill(location_mixing_matrix[i].begin(), location_mixing_matrix[i].end(),
         1.0);
  }
  // DEBUG
  /*std::cout << "nb_locations = " << nb_locations << std::endl;
  for (int i=0; i<nb_locations; i++){
    for (int j = 0; j<nb_locations; j++){
        std::cout << location_mixing_matrix[i][j] << ",";
    }
    std::cout<< std::endl;
  }*/ // END DEBUG
};

void SimParam::SetHivTransitionMatrix() {
  int nb_states = GemsState::kGemsLast;
  hiv_transition_matrix.clear();
  hiv_transition_matrix.resize(nb_states);

  int nb_years_categ = 7;

  for (int i = 0; i < nb_states; i++) {
    hiv_transition_matrix[i].resize(nb_years_categ);
    for (int j = 0; j < nb_years_categ; j++) {
      if (i == GemsState::kAcute) {
        // For all years and population categories
        hiv_transition_matrix[i][j].resize(nb_states);
        hiv_transition_matrix[i][j] = {
            0.0, 0.0, 1.0, 1.0, 1.0};  // After one year ACUTE, go to CHRONIC
      } else if (i == GemsState::kChronic) {
        if (j ==
            0) {  // Prior to 2003, for all (women 18-40, children and others)
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 1.0, 1.0,
                                         1.0};  // NO ART, then stay chronic
        } else if (j == 1) {  // Between to 2003 and 2010, for women 18-40
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.9, 1.0, 1.0};
        } else if (j == 2) {  // Between to 2003 and 2010, for children
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.8, 1.0, 1.0};
        } else if (j == 3) {  // Between to 2003 and 2010, for others
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.9, 1.0, 1.0};
        } else if (j == 4) {  // From 2011, for women 18-40
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.5, 1.0, 1.0};
        } else if (j == 5) {  // From 2011, for children
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.5, 1.0, 1.0};
        } else if (j == 6) {  // After 2011, for others
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.8, 1.0, 1.0};
        }
      } else if (i == GemsState::kTreated) {
        // For all years and population categories
        hiv_transition_matrix[i][j].resize(nb_states);
        hiv_transition_matrix[i][j] = {0.0, 0.0, 0.1, 1.0, 1.0};
      } else {
        hiv_transition_matrix[i][j].resize(nb_states);
        hiv_transition_matrix[i][j] = {0.0, 0.0, 0.0, 0.0, 0.0};
      }
    }
  }
};

}