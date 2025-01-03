// Mock implementation for core NVIC functions to avoid warnings
#ifdef TEST
#define SCB_VTOR_MOCK 0x20000000  // Mock base address for vector table

typedef struct {
    volatile uint32_t VTOR;
} SCB_Type;

SCB_Type SCB_Mock = {SCB_VTOR_MOCK};
#define SCB (&SCB_Mock)

void __NVIC_SetVector(IRQn_Type IRQn, uint32_t vector) {
    uint32_t *vectors = (uint32_t *)(uintptr_t)SCB->VTOR;  // Using uintptr_t for proper casting
    vectors[IRQn + 16] = vector;
}

uint32_t __NVIC_GetVector(IRQn_Type IRQn) {
    uint32_t *vectors = (uint32_t *)(uintptr_t)SCB->VTOR;  // Using uintptr_t for proper casting
    return vectors[IRQn + 16];
}
#endif

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include "main.h"

// Structure to track GPIO states
struct GPIOState {
    bool pins[16];
    uint32_t toggleCount[16];
    bool isInitialized;
    uint32_t errorCount;
    HAL_StatusTypeDef lastStatus;
};

// Global state tracking
static GPIOState gpioStates[3];
static uint32_t totalToggleCalls = 0;
static bool isSystemInitialized = false;
static uint32_t lastDelayValue = 0;
static HAL_StatusTypeDef lastOperationStatus = HAL_OK;

void resetGPIOState() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 16; j++) {
            gpioStates[i].pins[j] = false;
            gpioStates[i].toggleCount[j] = 0;
        }
        gpioStates[i].isInitialized = false;
        gpioStates[i].errorCount = 0;
        gpioStates[i].lastStatus = HAL_OK;
    }
    totalToggleCalls = 0;
    isSystemInitialized = false;
    lastDelayValue = 0;
    lastOperationStatus = HAL_OK;
}

// Mock GPIO functions
void mock_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    lastOperationStatus = HAL_OK;

    if (!GPIOx) {
        lastOperationStatus = HAL_ERROR;
        return;
    }

    int gpioIndex;
    if (GPIOx == GPIOA) gpioIndex = 0;
    else if (GPIOx == GPIOB) gpioIndex = 1;
    else if (GPIOx == GPIOC) gpioIndex = 2;
    else {
        lastOperationStatus = HAL_ERROR;
        return;
    }

    // Validate pin number
    if (GPIO_Pin > GPIO_PIN_15) {
        gpioStates[gpioIndex].errorCount++;
        lastOperationStatus = HAL_ERROR;
        return;
    }

    // Convert pin mask to index
    int pinIndex = 0;
    uint16_t mask = GPIO_Pin;
    while (mask > 1) {
        mask >>= 1;
        pinIndex++;
    }

    if (!gpioStates[gpioIndex].isInitialized) {
        gpioStates[gpioIndex].errorCount++;
        lastOperationStatus = HAL_ERROR;
        return;
    }

    // Check for overflow
    if (gpioStates[gpioIndex].toggleCount[pinIndex] == UINT32_MAX) {
        lastOperationStatus = HAL_ERROR;
        return;
    }

    gpioStates[gpioIndex].pins[pinIndex] = !gpioStates[gpioIndex].pins[pinIndex];
    gpioStates[gpioIndex].toggleCount[pinIndex]++;
    totalToggleCalls++;
}

// External C functions
extern "C" {
int some_function_from_main();

void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    mock_HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

void HAL_Delay(uint32_t Delay) {
    lastDelayValue = Delay;
}
}

// Test group
TEST_GROUP(LedBlinkGroup) {
                          void setup() {
                                       resetGPIOState();
for (int i = 0; i < 3; i++) {
    gpioStates[i].isInitialized = true;
}
}

void teardown() {
    resetGPIOState();
}
};

// Test cases
TEST(LedBlinkGroup, TestBasicToggle) {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
    CHECK_EQUAL(HAL_OK, lastOperationStatus);
    CHECK_EQUAL(1, gpioStates[1].toggleCount[4]);
}

TEST(LedBlinkGroup, TestInvalidPin) {
    HAL_GPIO_TogglePin(GPIOB, (uint16_t)0x8000);  // Pin beyond GPIO_PIN_15
    CHECK_EQUAL(HAL_ERROR, lastOperationStatus);
    CHECK_EQUAL(1, gpioStates[1].errorCount);
}

TEST(LedBlinkGroup, TestNullGPIO) {
    HAL_GPIO_TogglePin(NULL, GPIO_PIN_4);
    CHECK_EQUAL(HAL_ERROR, lastOperationStatus);
}

TEST(LedBlinkGroup, TestMultipleToggles) {
    const uint32_t TOGGLE_COUNT = 100;
    for (uint32_t i = 0; i < TOGGLE_COUNT; i++) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
        CHECK_EQUAL(HAL_OK, lastOperationStatus);
    }
    CHECK_EQUAL(TOGGLE_COUNT, gpioStates[1].toggleCount[4]);
}

TEST(LedBlinkGroup, TestPinSequence) {
    const uint16_t sequence[] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_8};
    const size_t seqLength = sizeof(sequence) / sizeof(sequence[0]);

    for (size_t i = 0; i < seqLength; i++) {
        HAL_GPIO_TogglePin(GPIOB, sequence[i]);
        CHECK_EQUAL(HAL_OK, lastOperationStatus);
    }

    for (size_t i = 0; i < seqLength; i++) {
        int pinIndex = 0;
        uint16_t mask = sequence[i];
        while (mask > 1) {
            mask >>= 1;
            pinIndex++;
        }
        CHECK_EQUAL(1, gpioStates[1].toggleCount[pinIndex]);
    }
}

int main(int argc, char** argv) {
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
