#include <Arduino.h>
#include "task3.h"
#include "events.h"

static bool compareKeys(BUTTONS *pSecret, BUTTONS *pKey)
{
    bool correct = true;
    for (uint8_t i = 0; i < 5; i++)
    {
        if (pSecret[i] != pKey[i])
        {
            correct = false;
            break;
        }
    }

    return correct;
}

void task3()
{
    enum class TaskStates
    {
        INIT,
        LENTO,
        MEDIO,
        RAPIDO,
        ENCENDIDO,
        APAGADO,
        ESPERA1,
        ESPERA2
    };
    static TaskStates taskState = TaskStates::LENTO;
    const uint8_t led = 11;

    static BUTTONS secret[5] = {BUTTONS::ONE_BTN, BUTTONS::ONE_BTN,
                                BUTTONS::TWO_BTN, BUTTONS::TWO_BTN,
                                BUTTONS::ONE_BTN};

    static BUTTONS disarmKey[5] = {BUTTONS::NONE};
    static uint32_t lastTime;
    static constexpr uint32_t INTERVALSLOW = 1000;
    static constexpr uint32_t INTERVALMEDIUM = 500;
    static constexpr uint32_t INTERVALFAST = 250;
    static uint8_t val;
    static uint8_t keyCounter;
    static bool ledStatus = false;

    switch (taskState)
    {
    case TaskStates::INIT:
    {

        pinMode(led, OUTPUT);
        ledStatus = true;
        digitalWrite(led, ledStatus);
        lastTime = millis();
        taskState = TaskStates::LENTO;
        break;
    }
    case TaskStates::LENTO:
    {
        uint32_t currentTime = millis();
        if ((currentTime - lastTime) >= INTERVALSLOW)
        {
            lastTime = currentTime;
            ledStatus = !ledStatus;
            digitalWrite(led, ledStatus);

            Serial.print("LENTO");
            Serial.print("\n");
        }
        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            if (buttonEvt.whichButton == BUTTONS::TWO_BTN)
            {
                taskState = TaskStates::MEDIO;
            }
            else if (buttonEvt.whichButton == BUTTONS::ONE_BTN)
            {
                taskState = TaskStates::ESPERA1;
            }
        }
        break;
    }

    case TaskStates::ESPERA1:
    {
        uint32_t currentTime = millis();
        if ((currentTime - lastTime) >= INTERVALSLOW)
        {
            ledStatus = false;
            digitalWrite(led, ledStatus);

            Serial.print("APAGADO ");
            Serial.print("\n");
            taskState = TaskStates::APAGADO;
        }
        break;
    }

    case TaskStates::MEDIO:
    {
        uint32_t currentTime = millis();
        if ((currentTime - lastTime) >= INTERVALMEDIUM)
        {
            lastTime = currentTime;
            ledStatus = !ledStatus;
            digitalWrite(led, ledStatus);

            Serial.print("MEDIO ");
            Serial.print("\n");
        }
        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            if (buttonEvt.whichButton == BUTTONS::TWO_BTN)
            {
                taskState = TaskStates::LENTO;
            }
            else if (buttonEvt.whichButton == BUTTONS::ONE_BTN)
            {
                taskState = TaskStates::ESPERA2;
            }
        }
        break;
    }

    case TaskStates::ESPERA2:
    {
        uint32_t currentTime = millis();
        if ((currentTime - lastTime) >= INTERVALSLOW)
        {
            taskState = TaskStates::ENCENDIDO;
        }
        break;
    }

    case TaskStates::ENCENDIDO:
    {
        digitalWrite(led, HIGH);
        ledStatus = true;
        Serial.print("ENCENDIDO ");
        Serial.print("\n");
        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            if (buttonEvt.whichButton == BUTTONS::ONE_BTN)
            {
                taskState = TaskStates::MEDIO;
            }
            else if (buttonEvt.whichButton == BUTTONS::TWO_BTN)
            {
                val = 1;
                Serial.print("RAPIDO");
                Serial.print("\n");
                taskState = TaskStates::RAPIDO;
            }
        }
        break;
    }

    case TaskStates::APAGADO:
    {

        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            if (buttonEvt.whichButton == BUTTONS::ONE_BTN)
            {
                digitalWrite(led, HIGH);
                ledStatus = true;
                lastTime = millis();
                taskState = TaskStates::LENTO;
            }
            else if (buttonEvt.whichButton == BUTTONS::TWO_BTN)
            {
                val = 2;
                digitalWrite(led, HIGH);
                ledStatus = true;
                Serial.print("RAPIDO");
                Serial.print("\n");
                taskState = TaskStates::RAPIDO;
            }
        }
        break;
    }
    case TaskStates::RAPIDO:
    {
        uint32_t currentTime = millis();
        if ((currentTime - lastTime) >= INTERVALFAST)
        {
            lastTime = currentTime;
            ledStatus = !ledStatus;
            digitalWrite(led, ledStatus);
            
        }
        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            disarmKey[keyCounter] = buttonEvt.whichButton;
            keyCounter++;
            if (keyCounter == 5)
            {
                keyCounter = 0;
                if (compareKeys(secret, disarmKey) == true)
                {
                    if (val == 2)
                    {
                        taskState = TaskStates::APAGADO;
                    }
                    else if (val == 1)
                    {
                        taskState = TaskStates::ENCENDIDO;
                    }
                }
            }
        }
        break;
    }
    }
}
