
#pragma once
#include "SFLA.h"
#include <vector>
#include <random>


std::vector<std::vector<Frog>> memplexes(NUMBER_OF_MEMPLEX);

std::vector<double> selection_probabilities;

std::vector<std::vector<Frog>> selected_frogs(NUMBER_OF_MEMPLEX);


int UgFitnessChange = 10000; // more accurate to condiser sigma of all knapsack items
Frog Ug;

#define NUMBER_OF_FROGS 6
#define NUMBER_OF_MEMPLEX 2
#define NUMBER_OF_ITEMS 9
#define Q_SELECTION 2
#define L_MAX_ITERATION 2
#define STEP_SIZE_MAX 1


void SFLA::start() {
    cout << "\n                           PHASE 0 _ PARAMETERS";
    cout << "\nNUMBER_OF_FROGS " << NUMBER_OF_FROGS;
    cout << "\nNUMBER_OF_MEMPLEX " << NUMBER_OF_MEMPLEX;
    cout << "\nNUMBER_OF_ITEMS " << NUMBER_OF_ITEMS;
    cout << "\nQ_SELECTION " << Q_SELECTION;
    cout << "\nL_MAX_ITERATION " << L_MAX_ITERATION;
    cout << "\nSTEP_SIZE_MAX " << STEP_SIZE_MAX;
    cout << "\n _______________________________________________________________";
    cout << "\n\n                 PHASE1 _ INIT FROGS, SET FITNESS AND SORT\n";

    receive_init_frogs();
    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        all_frogs[i].fitness = fitness_function(all_frogs[i].solution);
    }
    fitness_sorter(all_frogs);
    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        cout << "\nFitness:" << all_frogs[i].fitness << '\n';
        for (int j = 0; j < NUMBER_OF_ITEMS; ++j) {
            cout << all_frogs[i].solution[j] << " ";
        }
        cout << '\n';
    }

    cout << "_______________________________________________________________\n                   PHASE2 _ MEMPLEX PARTITION\n\n";

    //GMAX FROM HERE

    Ug = all_frogs[0];
    fitness_sorter(all_frogs);
    memplex_partition();


    cout << "_______________________________________________________________\n                   PHASE3 _ PROBABILITY CALCULATION\n";
    compute_selection_probabilities();

    cout << "_______________________________________________________________\n                   PHASE4 _ SELECT Q FROGS\n";

    //--------- Paralel for all i's
    select_q_frogs(0);
    fitness_sorter(selected_frogs[0]);
    //---------------------------------
    evolution_frog(0);
    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        //cout << "\nIndex:" << all_frogs[i].frog_index << "\nFitness:" << all_frogs[i].fitness << '\n';
        cout << "\nFitness:" << all_frogs[i].fitness << '\n';
        for (int j = 0; j < NUMBER_OF_ITEMS; ++j) {
            cout << all_frogs[i].solution[j] << " ";
        }
        cout << '\n';
    }

    //EVOLVE
}



void SFLA::receive_init_frogs() {
    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        Frog received_frog;
        frog_in.read(received_frog); // Read frog from FIFO
        all_frogs.push_back(received_frog);
    }
}


int SFLA::hamming_distance(sc_bv<NUMBER_OF_ITEMS>& U1, sc_bv<NUMBER_OF_ITEMS>& U2) {
    int distance = 0;
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        if (U1[i] != U2[i]) {
            distance++;
        }
    }
    return distance;
}


bool SFLA::updateUwBasedOnUb(int selected_id) {


    Frog& Uw = selected_frogs[selected_id][Q_SELECTION - 1];
    Frog& Ub = selected_frogs[selected_id][0];

    int random_number = 1;///TODO: ADD RAND TO STEP

    int maxStepSize = STEP_SIZE_MAX;
    int finalMaxStep = std::min(random_number * hamming_distance(Uw.solution, Ub.solution), maxStepSize);
    int curFitness = Uw.fitness;

    sc_bv<NUMBER_OF_ITEMS> newTestSolution;
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        newTestSolution[i] = Uw.solution[i];
    }

    int steps_used = 0;
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        if (Ub.solution[i] != Uw.solution[i]) {
            cout << i << " ";
            newTestSolution[i] = Ub.solution[i]; // Move towards U_B
            steps_used++;
            if (steps_used >= finalMaxStep) {
                cout << "\nMAX STEPS USED!";
                break;
            }
        }
    }

    int newFitness = fitness_function(newTestSolution);


    cout << "\nUb fitness and solution: " << Ub.fitness << "  -  " << Ub.solution;
    cout << "\nUw fitness and solution: " << Uw.fitness << "  -  " << Uw.solution;
    cout << "\nUWprime    and solution: " << newFitness << "  -  " << newTestSolution;

    if (newFitness > curFitness) {
        setupUwprime(selected_id, newTestSolution);
        cout << "\n         UB USED BY MEMPLEX " << selected_id;
        return true;
    }
    return false;
}


bool SFLA::updateUwBasedOnUg(int selected_id) {


    Frog& Uw = selected_frogs[selected_id][Q_SELECTION - 1];

    int random_number = 1;///TODO: ADD RAND TO STEP

    int maxStepSize = STEP_SIZE_MAX;
    int finalMaxStep = std::min(random_number * hamming_distance(Uw.solution, Ug.solution), maxStepSize);
    int curFitness = Uw.fitness;

    sc_bv<NUMBER_OF_ITEMS> newTestSolution; 
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        newTestSolution[i] = Uw.solution[i];
    }

    int steps_used = 0;
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        if (Ug.solution[i] != Uw.solution[i]) {
            cout <<"\n" << i << " ";
            newTestSolution[i] = Ug.solution[i]; // Move towards Ug
            steps_used++;
            if (steps_used >= finalMaxStep) {
                cout << "\nMAX STEPS USED!!";
                break;
            }
        }
    }

    int newFitness = fitness_function(newTestSolution);


    cout << "\nUb fitness and solution: " << Ug.fitness << "  -  " << Ug.solution;
    cout << "\nUw fitness and solution: " << Uw.fitness << "  -  " << Uw.solution;
    cout << "\nUWprime    and solution: " << newFitness << "  -  " << newTestSolution;

    if (newFitness > curFitness) {
        setupUwprime(selected_id, newTestSolution);
        cout << "\n         UG USED BY MEMPLEX " << selected_id; 
        return true;
    }
    return false;
}

void SFLA::evolution_frog(int selected_id) {
    sc_bv<NUMBER_OF_ITEMS> newSolution;
   
    int current_iteration = 0;

    do {
        fitness_sorter(selected_frogs[selected_id]);//selected_frogs[0] => Ub
        fitness_sorter(all_frogs); //all_frogs[0] => Ug

        Frog& Uw = selected_frogs[selected_id][Q_SELECTION - 1];
        cout << "\nold least is:" << Uw.fitness <<"  "<<Uw.solution << '\n';
        //part1 Uw and Ub
        
        if (updateUwBasedOnUb(selected_id)) {
        }
        
        //part2 Uw and Ug
        else if (updateUwBasedOnUg(selected_id)) {
        }

        //part3 Random
        else {
            for (int j = 0; j < NUMBER_OF_ITEMS; ++j) {
                newSolution[j] = (rand()) % 2;
                cout << "\n         RANDOM USED BY MEMPLEX: "<<selected_id;
                setupUwprime(selected_id, newSolution);
            }
        }
        current_iteration++;
        fitness_sorter(all_frogs); // if we have to update ug each time
        cout <<"\nUG FITNESS"<<Ug.fitness << "\n";
    } while (current_iteration <= L_MAX_ITERATION);
    fitness_sorter(all_frogs);
    //Alternate interp: set Ug = all_frogs[0] over here (just in case)
}

void SFLA::setupUwprime(int selected_id, sc_bv<NUMBER_OF_ITEMS> &newSolution) {
    Frog& Uw = selected_frogs[selected_id][Q_SELECTION - 1];

    Uw.solution = newSolution;
    Uw.fitness = fitness_function(newSolution);
    all_frogs[Uw.allfrogs_index] = Uw;
    cout << "\nNEW SOLUTION " << Uw.solution << " NEW FITNESS " << Uw.fitness;

}

int SFLA::fitness_function(sc_bv<NUMBER_OF_ITEMS> solution) {
    int value[NUMBER_OF_ITEMS] = { 8, 6, 3, 7, 6, 9, 8, 5, 6 };
    int weight[NUMBER_OF_ITEMS] = { 5, 4, 3, 9, 5, 7, 6, 3, 2 };
    int weight_limit = 20;
    int total_value = 0;
    int total_weight = 0;
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        if (solution[i] == 1) {
            total_value += value[i];
            total_weight += weight[i];
        }
    }
    if (total_weight > weight_limit) {
        return -1;
    }
    else {
        return total_value;
    }
}


void SFLA::fitness_sorter(std::vector<Frog>& frogs) {
    std::sort(std::begin(frogs), std::end(frogs), [](const Frog& firstFrog, const Frog& secondFrog) {
        return firstFrog.fitness > secondFrog.fitness;
        });
    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        all_frogs[i].allfrogs_index = i;
    }
    Ug = all_frogs[0];
}




void SFLA::memplex_partition() {
    for (auto& memplex : memplexes) {
        memplex.clear();
    }


    for (int i = 0; i < NUMBER_OF_FROGS; i++) {
        int memplex_id = i % NUMBER_OF_MEMPLEX;
        all_frogs[i].memplex_index = memplex_id;
        all_frogs[i].memplex_offset = memplexes[memplex_id].size();
        memplexes[memplex_id].push_back(all_frogs[i]);
    }
    for (int i = 0; i < NUMBER_OF_MEMPLEX; i++) {
        for (int j = 0; j < NUMBER_OF_FROGS / NUMBER_OF_MEMPLEX; j++) {
                std::cout << "[" << memplexes[i][j].fitness << ", " << i << ", " << j << "] Solution: " << memplexes[i][j].solution<<"\n";

        }
    }
}



void SFLA::compute_selection_probabilities() {
    int n = NUMBER_OF_FROGS / NUMBER_OF_MEMPLEX; // size of each memplex

    selection_probabilities.clear();
    double sum_pn = 0;

    for (int j = 1; j <= n; j++) {
        double p_n = (2.0 * (n + 1 - j)) / (n * (n + 1));
        selection_probabilities.push_back(p_n);
        sum_pn += p_n;
    }
    //normalize
    for (double& p : selection_probabilities) {
        p /= sum_pn;
    }
    cout << "Shared Probabilities: ";
    for (double p : selection_probabilities) {
        cout << p << " ";
    }
    cout << '\n';
}

// --------------------
//TODO: HATMAN BE RESET KARDAN SELECTED FROGS HAVASEMUN BASHE 
// --------------------
void SFLA::select_q_frogs(int memplex_id) {
    int n = NUMBER_OF_FROGS / NUMBER_OF_MEMPLEX;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<int> dist(&selection_probabilities[0], &selection_probabilities[0] + n);

    bool selected[NUMBER_OF_FROGS / NUMBER_OF_MEMPLEX] = { false };

    std::cout << "Selected frogs from Memplex " << memplex_id << ": ";

    for (int i = 0; i < Q_SELECTION; i++) {
        int selected_index;
        do {
            selected_index = dist(gen);
        } while (selected[selected_index]); // Ensure unique selection

        selected[selected_index] = true; // Mark this frog as selected
        selected_frogs[memplex_id].push_back(memplexes[memplex_id][selected_index]);

        std::cout << "[Fitness: " << selected_frogs[memplex_id][i].fitness
            << ", Offset: " << selected_frogs[memplex_id][i].memplex_offset << "] ";
    }
    std::cout << std::endl;
}
