
#include "vehicle_model.hpp"
#include <cstdio>

int main() {
    
    // Run a vehicle simulation for 10 seconds
    VehicleState s;
    for (int i = 0; i < 10; i++) {
        update_vehicle(s, 1.0);
        printf("t=%2d speed=%6.1f rpm=%6.0f coolant=%5.1f\n",
            i, s.speed_kmh, s.rpm, s.coolant_c);
    }

    return 0;
}