#include "sim-param.h"

namespace bdm {
namespace hiv_malawi {

// Initialize parameter group Uid, part of the BioDynaMo API, needs to be part
// of a cc file, depends on #include "sim-param.h". With this, we can access the
// simulation parameters anywhere in the simulation.
const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

void SimParam::SetSociobehavMixingMatrix() {
  sociobehav_mixing_matrix.clear();
  sociobehav_mixing_matrix.resize(nb_age_categories);

  for (int i = 0; i < nb_sociobehav_categories; i++) {
    sociobehav_mixing_matrix[i].resize(nb_sociobehav_categories);
    // Fill all elements with 1.0. Homogeneous socio-behavioural mixing.
    fill(sociobehav_mixing_matrix[i].begin(), sociobehav_mixing_matrix[i].end(),
         1.0);
  }
};

void SimParam::SetRegPartnerSociobehavMixingMatrix() {
  reg_partner_sociobehav_mixing_matrix.clear();
  reg_partner_sociobehav_mixing_matrix.resize(nb_age_categories);

  for (int i = 0; i < nb_sociobehav_categories; i++) {
    reg_partner_sociobehav_mixing_matrix[i].resize(nb_sociobehav_categories);
    // Fill all elements with 1.0. Homogeneous socio-behavioural mixing.
    fill(reg_partner_sociobehav_mixing_matrix[i].begin(),
         reg_partner_sociobehav_mixing_matrix[i].end(), 1.0);
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

void SimParam::SetRegPartnerAgeMixingMatrix() {
  reg_partner_age_mixing_matrix.clear();
  reg_partner_age_mixing_matrix.resize(nb_age_categories);

  for (int i = 0; i < nb_age_categories; i++) {
    reg_partner_age_mixing_matrix[i].resize(nb_age_categories);
    // Fill all elements with 1.0. Homogeneous age mixing.
    fill(reg_partner_age_mixing_matrix[i].begin(),
         reg_partner_age_mixing_matrix[i].end(), 1.0);
  }
};

void SimParam::SetLocationMixingMatrix() {
  location_mixing_matrix.clear();
  location_mixing_matrix.resize(nb_locations);

  for (int i = 0; i < nb_locations; i++) {
    location_mixing_matrix[i].resize(nb_locations);
    // Fill all elements with 0.0 except diagonal with 1.0.
    fill(location_mixing_matrix[i].begin(), location_mixing_matrix[i].end(),
         0.0);
    location_mixing_matrix[i][i] = 1.0;
    // Fill all elements with 1.0 (Homogeneous mixing)
    /*fill(location_mixing_matrix[i].begin(), location_mixing_matrix[i].end(),
         1.0);*/
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

/*int SimParam::ComputeYearPopulationCategory(int year, float age, int sex){
    int year_population_category;
    if (year < 2003) {  // Prior to 2003
      year_population_category = 0;  // All (No difference in ART between
people. ART not available.) } else if (year < 2011) {  // Between 2003 and 2010
      if (sex == Sex::kFemale and age >= 18 and age <= 40) {
        year_population_category = 1;  // Female between 18 and 40
      } else if (age < 15) {
        year_population_category = 2;  // Child
      } else {
        year_population_category =
            3;  // Others (Male, Female under 18, and Female over 40)
      }
    } else {  // After 2011
      if (sex == Sex::kFemale and age >= 18 and age <= 40) {
        year_population_category = 4;  // Female between 18 and 40
      } else if (age < 15) {
        year_population_category = 5;  // Child
      } else {
        year_population_category = 6;  // Others (Male, Female under 18, and
Female over 40)
      }
    }
    return year_population_category;
}*/

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
            0) {  // Prior to 2003, for all (women 15-40, children and others)
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 1.0, 1.0,
                                         1.0};  // NO ART, then stay chronic
        } else if (j == 1) {  // Between to 2003 and 2010, for women 15-40
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.9, 1.0, 1.0};
        } else if (j == 2) {  // Between to 2003 and 2010, for children
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.8, 1.0, 1.0};
        } else if (j == 3) {  // Between to 2003 and 2010, for others
          hiv_transition_matrix[i][j].resize(nb_states);
          hiv_transition_matrix[i][j] = {0.0, 0.0, 0.9, 1.0, 1.0};
        } else if (j == 4) {  // From 2011, for women 15-40
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

void SimParam::SetMigrationMatrix() {
  migration_matrix.clear();
  int nb_migration_year_transitions = migration_year_transition.size();
  migration_matrix.resize(nb_migration_year_transitions);
  for (int y = 0; y < nb_migration_year_transitions; y++) {
    migration_matrix[y].resize(nb_locations);
    for (int i = 0; i < nb_locations; i++) {
      migration_matrix[y][i].resize(nb_locations);
      // Fill all elements with 1.0 except diagonal with 0.0.
      fill(migration_matrix[y][i].begin(), migration_matrix[y][i].end(), 1.0);
      migration_matrix[y][i][i] = 0.0;
    }
  }
};

void SimParam::SetInitialInfectionProbability() {
  // Compute Proportion of Population in Seed Districts
  float districts_proportion = 0.0;
  int nb_seed_districts = 0;
  for (size_t i = 0; i < nb_locations; i++) {
    if (seed_districts[i] == true) {
      nb_seed_districts += 1;
      if (i == 0) {
        districts_proportion += location_distribution[i];
      } else {
        districts_proportion +=
            (location_distribution[i] - location_distribution[i - 1]);
      }
    }
  }

  std::cout << "nb_seed_districts = " << nb_seed_districts
            << "; districts_proportion = " << districts_proportion << std::endl;

  // Given Proportion of Seed District Populations and National Prevalence
  // Probability to be Healthy is 1 - Probability to be Infected
  initial_healthy_probability =
      (1.0 - initial_prevalence / districts_proportion);
};

}  // namespace hiv_malawi
}  // namespace bdm
