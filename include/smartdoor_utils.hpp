#pragma once

#include <cstdint>
#include "smartdoor_types.hpp"
#include "door_controller.hpp"

Command decodeCommand(uint8_t value);

const char* toString(Command cmd);
const char* toString(LockState state);
const char* toString(DoorState state);
const char* toString(LampState state);
const char* toString(FaultState state);

uint8_t toByte(LockState state);
uint8_t toByte(DoorState state);
uint8_t toByte(LampState state);
uint8_t toByte(FaultState state);

void printState(const DoorController& controller);