# Voice Recognition Application User Guide

## 1. Introduction
This application is a voice recognition tool developed based on Maix-Speech. It integrates three core voice processing capabilities: digit recognition, keyword wake-up, and long-form continuous speech recognition (LVCSR). It displays recognition results through a visual interface and supports touch operation to switch between functional modes, making it suitable for Chinese voice interaction scenarios.

## 2. Main Features

| Feature Module | Description |
|----------|----------|
| **Digit Recognition** | Recognizes numeric content in speech and displays the results in real-time on the screen. |
| **Keyword Wake-up (KWS)** | Supports recognition of three preset keywords: "Xiaoai Tongxue" (Xiaomi AI), "Tmall Genie", and "Tianqi Zenmeyang" (How is the weather). The recognized keyword is highlighted upon detection. |
| **Long-form Continuous Speech Recognition (LVCSR)** | Supports general continuous Chinese speech recognition. It can recognize arbitrary Chinese sentences and outputs both Pinyin and Hanzi (Chinese character) results. |
| **Touch Interaction** | Intuitive and convenient operation to switch between different functional modes and clear recognition results via screen touch. |
| **Multilingual Interface** | Supports switching between Chinese and English interfaces to adapt to different language usage habits. |

## 3. Usage Instructions

### 3.1 Starting the Application
After deploying the application to the Maix hardware device, run the executable file to start it. Upon launch, the voice recognition main interface will appear, and the app will default to **Digit Recognition mode** (indicated by the green "digit" label at the bottom).

### 3.2 Switching Functions
Switch modes by tapping the function area at the bottom of the screen:

*   **Clear**: Tap the "Clear" area at the bottom left to clear the recognition results currently displayed on the screen.
*   **Digit**: Tap the "digit" area on the bottom left to switch to Digit Recognition mode (the label turns green). Speak numbers to have them recognized and displayed.
*   **KWS**: Tap the "kws" area in the bottom center to switch to Keyword Wake-up mode (the label turns green). The screen will display the three keywords: "Xiaoai Tongxue", "Tmall Genie", and "Tianqi Zenmeyang". Speaking the corresponding keyword will trigger recognition and highlight the word.
*   **LVCSR**: Tap the "lvcsr" area on the bottom right to switch to Long-form Speech Recognition mode (the label turns green). Speak any Chinese sentence, and the screen will display the recognized text in both Hanzi and Pinyin.

### 3.3 Exiting the Application
Tap the exit area in the top-left corner of the screen (approx. 40×40 pixels) to close the application.

## 4. Notes

1.  **Model Dependencies**: Before running the application, ensure the required voice recognition model files exist in the `/root/models/` directory (specifically `am_3332_192_int8.mud` and language models under the `lmS` directory). Missing models will prevent the functions from working.
2.  **Operational Tips**: When switching functional modes, wait for the current mode to stop running before tapping to avoid functional conflicts.
3.  **Voice Input**: For optimal accuracy, maintain a quiet environment and speak at a distance of 30-50cm from the microphone.

## 5. More Information

*  [Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_speech)
*  [MaixCAM MaixPy Real-time Speech Recognition](https://wiki.sipeed.com/maixpy/doc/en/audio/recognize.html)