#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath> 

enum class GameState {
    MainMenu,
    DarkNetShop,
    IntroOverlay,
    Gameplay,
    RoundStats,
    EscapeConfirm,
    EmergencyConsole,
    GameOver,
    Victory,
    LoreInstructions
};

enum class Difficulty { Easy, Normal, Hard };

enum class Modifier {
    None,
    Snitch,
    Deadline,
    Masking,
    Backdoor,
    Interpol
};

struct SystemLeaks {
    std::wstring knownDigits = L"? # ? ?";
    int sumOfDigits = 0;
    std::wstring missingDigits = L"Нет";
};

struct CyberAbilities {
    bool hasCloudFirewall = false;
    bool hasActivator = false;
};

std::wstring generateSecretCode(Difficulty diff) {
    std::wstring code = L"";
    if (diff == Difficulty::Easy) {
        bool used[10] = { false };
        while (code.length() < 4) {
            int digit = std::rand() % 10;
            if (!used[digit]) { code += std::to_wstring(digit); used[digit] = true; }
        }
    }
    else {
        for (int i = 0; i < 4; ++i) code += std::to_wstring(std::rand() % 10);
    }
    return code;
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "HACK THIS NEVADA LAB!", sf::Style::Close);
    window.setFramerateLimit(60);

    // --- ЗАГРУЗКА РЕСУРСОВ ---
    sf::Font font;
    if (!font.loadFromFile("cour.ttf")) {
        std::wcout << L"Ошибка! Положите cour.ttf рядом с exe" << std::endl;
    }

    sf::Texture bgNormalTexture;
    sf::Texture bgEscapeTexture;
    sf::Sprite bgSprite;

    bool hasNormalBg = bgNormalTexture.loadFromFile("background.png");
    bool hasEscapeBg = bgEscapeTexture.loadFromFile("background_escape.png");

    if (hasNormalBg) {
        bgSprite.setTexture(bgNormalTexture);
        bgSprite.setScale(1920.0f / bgNormalTexture.getSize().x, 1080.0f / bgNormalTexture.getSize().y);
    }

    sf::Music ambientMusic;
    sf::Music alarmMusic;
    sf::SoundBuffer clickBuffer;
    sf::Sound clickSound;

    if (ambientMusic.openFromFile("ambient.ogg")) {
        ambientMusic.setLoop(true);
        ambientMusic.setVolume(40.0f);
        ambientMusic.play();
    }
    if (alarmMusic.openFromFile("alarm.ogg")) {
        alarmMusic.setLoop(true);
        alarmMusic.setVolume(45.0f);
    }
    if (clickBuffer.loadFromFile("click.wav")) {
        clickSound.setBuffer(clickBuffer);
        clickSound.setVolume(30.0f);
    }

    // --- ИГРОВЫЕ ПЕРЕМЕННЫЕ ---
    GameState currentState = GameState::MainMenu;
    Difficulty currentDiff = Difficulty::Normal;
    std::vector<Modifier> activeModifiers;
    CyberAbilities skills;

    std::wstring secretCode = L"";
    std::wstring userInput = L"";
    std::wstring modAlertText = L"";

    float currentRoundCash = 15000.0f;
    float displayedCash = 15000.0f;
    float totalWallet = 1000.0f;
    float earnedThisRound = 0.0f;

    int currentRound = 1;
    const int TOTAL_ROUNDS = 3;
    int wrongAttemptsCount = 0;

    sf::Clock gameClock;
    sf::Clock introClock;
    sf::Clock autoLeakClock;

    float leakInterval = 12.0f;
    float cashDrainSpeed = 40.0f;
    float baseCashDrainSpeed = 40.0f;

    SystemLeaks leaks;
    std::vector<wchar_t> wrongDigits;
    const std::wstring noiseSymbols = L"%№*#@&$!\\/";

    // --- ПЕРЕМЕННЫЕ АНИМАЦИИ ---
    float introAlpha = 190.0f;
    float typewriterTimer = 0.0f;
    size_t typewriterLength = 0;
    bool isRoundWon = false;
    float winFreezeTimer = 0.0f;
    float escapeBlinkTimer = 0.0f;
    float confirmBoxAlpha = 0.0f;
    float escapeTypewriterTimer = 0.0f;
    float menuAnimationTimer = 0.0f;

    // --- РЕЖИМ ПОБЕГА ---
    float escapeTimer = 600.0f;
    int escapeCodesNeeded = 3;
    int escapeCodesCracked = 0;

    // --- ИНТЕРФЕЙС ТЕРМИНАЛА ---
    sf::RectangleShape leftLogRect(sf::Vector2f(720, 980));
    leftLogRect.setFillColor(sf::Color(5, 15, 5, 220));
    leftLogRect.setOutlineColor(sf::Color(0, 150, 0, 150));
    leftLogRect.setOutlineThickness(2);
    leftLogRect.setPosition(40, 50);

    sf::Text leftLogText(L"", font, 24);
    leftLogText.setFillColor(sf::Color(0, 240, 0));
    leftLogText.setPosition(60, 65);

    sf::Text mainText(L"", font, 36);
    mainText.setFillColor(sf::Color(0, 255, 0));
    mainText.setPosition(250, 400);

    sf::RectangleShape hackWindowRect(sf::Vector2f(750, 500));
    hackWindowRect.setFillColor(sf::Color(5, 20, 5, 230));
    hackWindowRect.setOutlineColor(sf::Color(0, 200, 0));
    hackWindowRect.setOutlineThickness(3);
    hackWindowRect.setPosition(1100, 250);

    sf::Text hackWindowTitle(L" HACKNEVADA v7.7 // QUANTUM CORE DECRYPTOR", font, 22);
    hackWindowTitle.setFillColor(sf::Color(0, 220, 0));
    hackWindowTitle.setPosition(1100, 210);

    sf::Text hackCodeText(L"&&&&", font, 130);
    hackCodeText.setFillColor(sf::Color(255, 40, 40));
    hackCodeText.setPosition(1250, 400);

    sf::Text activeSkillsText(L"", font, 22);
    activeSkillsText.setPosition(1100, 100);

    sf::RectangleShape escapeBtnRect(sf::Vector2f(220, 60));
    escapeBtnRect.setFillColor(sf::Color(15, 5, 5, 200));
    escapeBtnRect.setOutlineColor(sf::Color::Red);
    escapeBtnRect.setOutlineThickness(3);
    escapeBtnRect.setPosition(1100, 780);

    sf::Text escapeBtnText(L" [ ПОБЕГ ] ", font, 28);
    escapeBtnText.setFillColor(sf::Color::Red);
    escapeBtnText.setPosition(1135, 792);

    sf::RectangleShape confirmBox(sf::Vector2f(650, 250));
    confirmBox.setFillColor(sf::Color(25, 5, 5, 245));
    confirmBox.setOutlineColor(sf::Color::Red);
    confirmBox.setOutlineThickness(4);
    confirmBox.setPosition(635, 415);

    sf::Text confirmText(L" ИНИЦИИРОВАТЬ ЭКСТРЕННЫЙ СБРОС?\n\n [Y] - ПЕРЕЙТИ В КОРНЕВОЙ ТЕРМИНАЛ\n [N] - ВЕРНУТЬСЯ К ЯДРУ", font, 24);
    confirmText.setPosition(660, 460);

    sf::Text consoleText(L"", font, 26);
    consoleText.setFillColor(sf::Color(0, 255, 100));
    consoleText.setPosition(100, 100);

    // Главное Меню
    sf::Text menuTitle(L" NEVADA LABS // QUANTUM METAFRAME OS", font, 46);
    menuTitle.setFillColor(sf::Color(0, 255, 0));
    menuTitle.setPosition(400, 220);

    sf::Text startBtn(L"[ 1 ] ИНИЦИАЛИЗИРОВАТЬ ВЗЛОМ HACKNEVADA", font, 32);
    startBtn.setPosition(520, 450);

    sf::Text diffBtn(L"[ 2 ] КВАНТОВАЯ ПЛОТНОСТЬ ЯДРА: MIDDLE", font, 32);
    diffBtn.setPosition(520, 530);

    sf::Text loreBtn(L"[ 3 ] ИНСТРУКЦИЯ НЕТРАННЕРА", font, 32);
    loreBtn.setPosition(520, 610);

    sf::Text menuFooter(L"Используйте мышь или клавиши [1], [2], [3] на клавиатуре", font, 22);
    menuFooter.setFillColor(sf::Color(100, 150, 100));
    menuFooter.setPosition(580, 750);

    // Окно Инструкции
    sf::Text loreTitle(L"--- БОРТОВОЙ СПРАВОЧНИК КИБЕР-НАЛЕТЧИКА ---", font, 40);
    loreTitle.setFillColor(sf::Color(0, 200, 255));
    loreTitle.setPosition(350, 80);

    sf::Text loreContentText(L"", font, 24);
    loreContentText.setFillColor(sf::Color(200, 255, 200));
    loreContentText.setPosition(150, 180);

    // ЭЛЕМЕНТЫ ЧЕРНОГО РЫНКА
    sf::Text shopTitle(L"   --- BLACK MARKET: СОФТ ДЛЯ ОБХОДА QUANTUM OS ---", font, 42);
    shopTitle.setFillColor(sf::Color(255, 30, 30));
    shopTitle.setPosition(200, 60);

    sf::Text shopWalletText(L"", font, 24);
    shopWalletText.setFillColor(sf::Color::White);
    shopWalletText.setPosition(200, 150);

    sf::Text sItem1(L"[ 1 ] КУПИТЬ МАСКИРОВКУ (Снижение убыли денег на ОДИН раунд) - $600", font, 24);
    sf::Text sItem2(L"[ 2 ] КУПИТЬ АКТИВНЫЙ 'ОБЛАЧНЫЙ ФАЕРВОЛЛ [Z]' (Сброс дебаффа) - $600", font, 24);
    sf::Text sItem3(L"[ 3 ] КУПИТЬ АКТИВНЫЙ СКРИПТ 'СИСТЕМНЫЙ АКТИВАТОР [X]' - $500", font, 24);

    sItem1.setPosition(200, 320);
    sItem2.setPosition(200, 400);
    sItem3.setPosition(200, 480);

    sf::Text itemSkipBtn(L"[ Нажмите SPACE для прямого подключения к ноде мейнфрейма ]", font, 30);
    itemSkipBtn.setFillColor(sf::Color::Yellow);
    itemSkipBtn.setPosition(450, 850);

    // ЭЛЕМЕНТЫ ОВЕРЛЕЯ ЗАГРУЗКИ
    sf::RectangleShape introOverlayBox(sf::Vector2f(1600, 550));
    introOverlayBox.setPosition(160, 260);

    sf::Text introOverlayText(L"", font, 34);
    introOverlayText.setPosition(220, 320);

    // ЭЛЕМЕНТЫ ОКНА СТАТИСТИКИ РАУНДА
    sf::RectangleShape statsBoxRect(sf::Vector2f(1200, 600));
    statsBoxRect.setFillColor(sf::Color(5, 25, 10, 245));
    statsBoxRect.setOutlineColor(sf::Color(0, 255, 0));
    statsBoxRect.setOutlineThickness(4);
    statsBoxRect.setPosition(360, 240);

    sf::Text statsTitleText(L"=== ОТЧЕТ ДЕШИФРАЦИИ ЯДРА МЕЙНФРЕЙМА ===", font, 38);
    statsTitleText.setFillColor(sf::Color::Yellow);
    statsTitleText.setPosition(440, 280);

    sf::Text statsContentText(L"", font, 28);
    statsContentText.setFillColor(sf::Color::White);
    statsContentText.setPosition(440, 380);

    auto rebuildModifierText = [&](const std::vector<Modifier>& mods) -> std::wstring {
        std::wstring text = L"";
        for (Modifier m : mods) {
            if (m == Modifier::Snitch) text += L"[ФАЙРВОЛ СТУКАЧ] ";
            if (m == Modifier::Deadline) text += L"[ДЕДЛАЙН x3] ";
            if (m == Modifier::Masking) text += L"[ОДНОРАЗОВАЯ МАСКИРОВКА] ";
            if (m == Modifier::Interpol) text += L"[ПЕРЕХВАТ ИНТЕРПОЛА х4] ";
            if (m == Modifier::Backdoor) text += L"[БЭКДОР] ";
        }
        return text.empty() ? L"НЕСТАБИЛЬНОСТЬ СЕТИ: НЕ НАЙДЕНА" : text;
        };

    auto setupEmergencyConsole = [&]() {
        escapeCodesCracked = 0;
        userInput = L"";
        escapeTypewriterTimer = 0.0f;
        secretCode = generateSecretCode(currentDiff);
        escapeTimer = 600.0f;

        if (currentDiff == Difficulty::Easy)         escapeCodesNeeded = 2;
        else if (currentDiff == Difficulty::Normal)   escapeCodesNeeded = 3;
        else                                         escapeCodesNeeded = 4;

        if (hasEscapeBg) {
            bgSprite.setTexture(bgEscapeTexture);
            bgSprite.setScale(1920.0f / bgEscapeTexture.getSize().x, 1080.0f / bgEscapeTexture.getSize().y);
        }

        ambientMusic.setVolume(20.0f);
        alarmMusic.setVolume(45.0f);
        alarmMusic.play();

        currentState = GameState::EmergencyConsole;
        };

    auto prepareRound = [&]() {
        secretCode = generateSecretCode(currentDiff);
        std::wcout << L"[DEBUG] Квантовый ключ: " << secretCode << std::endl;

        userInput = L"";
        wrongDigits.clear();
        leaks = SystemLeaks();
        currentRoundCash = 15000.0f;
        displayedCash = 15000.0f;
        wrongAttemptsCount = 0;
        typewriterTimer = 0.0f;
        typewriterLength = 0;
        isRoundWon = false;
        winFreezeTimer = 0.0f;

        if (currentDiff == Difficulty::Easy) baseCashDrainSpeed = 25.0f;
        else if (currentDiff == Difficulty::Normal) baseCashDrainSpeed = 50.0f;
        else baseCashDrainSpeed = 90.0f;

        cashDrainSpeed = baseCashDrainSpeed;
        leakInterval = 12.0f;

        // Удаляем маскировку из списка модификаторов, так как она была одноразовой для прошлого раунда
        activeModifiers.erase(std::remove(activeModifiers.begin(), activeModifiers.end(), Modifier::Masking), activeModifiers.end());

        int r = std::rand() % 5;
        if (r == 1) activeModifiers.push_back(Modifier::Snitch);
        if (r == 2) activeModifiers.push_back(Modifier::Deadline);
        if (r == 3) activeModifiers.push_back(Modifier::Backdoor);

        currentState = GameState::DarkNetShop;
        };

    auto toggleDifficulty = [&]() {
        if (currentDiff == Difficulty::Easy) { currentDiff = Difficulty::Normal; diffBtn.setString(L"[ 2 ] КВАНТОВАЯ ПЛОТНОСТЬ ЯДРА: MIDDLE"); }
        else if (currentDiff == Difficulty::Normal) { currentDiff = Difficulty::Hard; diffBtn.setString(L"[ 2 ] КВАНТОВАЯ ПЛОТНОСТЬ ЯДРА: K-MAX"); }
        else { currentDiff = Difficulty::Easy; diffBtn.setString(L"[ 2 ] КВАНТОВАЯ ПЛОТНОСТЬ ЯДРА: LOW"); }
        };

    auto launchGameplayAfterShop = [&]() {
        bool hasDeadline = false;
        bool hasMasking = false;
        bool hasInterpol = false;

        bool triggeredInterpol = false;
        for (Modifier m : activeModifiers) {
            if (m == Modifier::Masking) triggeredInterpol = true;
        }
        if (triggeredInterpol && (std::rand() % 100) < 50) {
            activeModifiers.push_back(Modifier::Interpol);
        }

        for (Modifier mod : activeModifiers) {
            if (mod == Modifier::Deadline) hasDeadline = true;
            if (mod == Modifier::Masking) hasMasking = true;
            if (mod == Modifier::Interpol) hasInterpol = true;
            if (mod == Modifier::Backdoor) {
                if ((std::rand() % 100) < 70) {
                    leaks.knownDigits[0] = secretCode[0];
                }
                else {
                    leakInterval = 9999.0f;
                }
            }
        }

        cashDrainSpeed = baseCashDrainSpeed;
        if (hasDeadline) cashDrainSpeed *= 3.0f;
        if (hasMasking) cashDrainSpeed *= 0.25f;
        if (hasInterpol) { cashDrainSpeed *= 4.0f; leakInterval = 9999.0f; }

        modAlertText = rebuildModifierText(activeModifiers);

        int tempSum = 0;
        for (int i = 0; i < 4; i++) tempSum += secretCode[i] - L'0';
        leaks.sumOfDigits = tempSum;

        introAlpha = 190.0f;
        introClock.restart();
        autoLeakClock.restart();
        currentState = GameState::IntroOverlay;
        };

    auto resetToNormalTheme = [&]() {
        alarmMusic.stop();
        ambientMusic.setVolume(40.0f);
        if (ambientMusic.getStatus() != sf::SoundSource::Playing) {
            ambientMusic.play();
        }
        if (hasNormalBg) {
            bgSprite.setTexture(bgNormalTexture);
            bgSprite.setScale(1920.0f / bgNormalTexture.getSize().x, 1080.0f / bgNormalTexture.getSize().y);
        }
        };

    // --- ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ ---
    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (currentState == GameState::MainMenu) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (startBtn.getGlobalBounds().contains(mousePosF)) {
                        clickSound.play(); currentRound = 1; totalWallet = 1000.0f;
                        skills = CyberAbilities();
                        prepareRound();
                    }
                    if (diffBtn.getGlobalBounds().contains(mousePosF)) {
                        clickSound.play(); toggleDifficulty();
                    }
                    if (loreBtn.getGlobalBounds().contains(mousePosF)) {
                        clickSound.play(); currentState = GameState::LoreInstructions;
                    }
                }
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1) {
                        clickSound.play(); currentRound = 1; totalWallet = 1000.0f;
                        skills = CyberAbilities();
                        prepareRound();
                    }
                    if (event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2) {
                        clickSound.play(); toggleDifficulty();
                    }
                    if (event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Numpad3) {
                        clickSound.play(); currentState = GameState::LoreInstructions;
                    }
                }
            }
            else if (currentState == GameState::LoreInstructions) {
                if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed) {
                    clickSound.play();
                    currentState = GameState::MainMenu;
                }
            }
            else if (currentState == GameState::DarkNetShop) {
                if (event.type == sf::Event::KeyPressed) {
                    if ((event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1) && totalWallet >= 600.0f) {
                        clickSound.play(); totalWallet -= 600.0f;
                        activeModifiers.push_back(Modifier::Masking);
                    }
                    if ((event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2) && totalWallet >= 600.0f && !skills.hasCloudFirewall) {
                        clickSound.play(); totalWallet -= 600.0f;
                        skills.hasCloudFirewall = true;
                    }
                    if ((event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Numpad3) && totalWallet >= 500.0f && !skills.hasActivator) {
                        clickSound.play(); totalWallet -= 500.0f;
                        skills.hasActivator = false;
                    }
                    if (event.key.code == sf::Keyboard::Space) {
                        clickSound.play(); launchGameplayAfterShop();
                    }
                }
            }
            else if (currentState == GameState::Gameplay && !isRoundWon) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Z && skills.hasCloudFirewall) {
                        std::vector<Modifier> badMods;
                        for (Modifier m : activeModifiers) {
                            if (m == Modifier::Snitch || m == Modifier::Deadline || m == Modifier::Interpol) {
                                badMods.push_back(m);
                            }
                        }

                        if (!badMods.empty()) {
                            clickSound.play();
                            Modifier targetToKill = badMods[std::rand() % badMods.size()];

                            activeModifiers.erase(std::remove(activeModifiers.begin(), activeModifiers.end(), targetToKill), activeModifiers.end());
                            skills.hasCloudFirewall = false;

                            bool hasDeadline = false;
                            bool hasMasking = false;
                            bool hasInterpol = false;
                            for (Modifier mod : activeModifiers) {
                                if (mod == Modifier::Deadline) hasDeadline = true;
                                if (mod == Modifier::Masking) hasMasking = true;
                                if (mod == Modifier::Interpol) hasInterpol = true;
                            }

                            cashDrainSpeed = baseCashDrainSpeed;
                            if (hasDeadline) cashDrainSpeed *= 3.0f;
                            if (hasMasking) cashDrainSpeed *= 0.25f;
                            if (hasInterpol) cashDrainSpeed *= 4.0f; else if (targetToKill == Modifier::Interpol) leakInterval = 12.0f;

                            modAlertText = rebuildModifierText(activeModifiers);
                        }
                    }
                    if (event.key.code == sf::Keyboard::X && skills.hasActivator) {
                        std::vector<int> closedPositions;
                        for (int i = 0; i < 4; i++) { if (leaks.knownDigits[i * 2] == L'?') closedPositions.push_back(i); }

                        if (!closedPositions.empty()) {
                            clickSound.play();
                            int rp = closedPositions[std::rand() % closedPositions.size()];
                            leaks.knownDigits[rp * 2] = secretCode[rp];
                            currentRoundCash *= 0.80f;
                            skills.hasActivator = true;
                        }
                    }
                }

                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode >= '0' && event.text.unicode <= '9' && userInput.length() < 4) {
                        clickSound.play();
                        userInput += static_cast<wchar_t>(event.text.unicode);
                    }
                    if (event.text.unicode == 8 && !userInput.empty()) {
                        clickSound.play();
                        userInput.pop_back();
                    }
                }
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                    if (userInput.length() == 4) {
                        if (userInput == secretCode) {
                            isRoundWon = true;
                            winFreezeTimer = 0.0f;
                            earnedThisRound = currentRoundCash;
                        }
                        else {
                            wrongAttemptsCount++;

                            if (std::find(activeModifiers.begin(), activeModifiers.end(), Modifier::Snitch) != activeModifiers.end() && wrongAttemptsCount >= 3) {
                                wrongAttemptsCount = 0;
                                std::vector<int> openPositions;
                                for (int i = 0; i < 4; ++i) { if (leaks.knownDigits[i * 2] != L'?') openPositions.push_back(i); }
                                if (!openPositions.empty()) { leaks.knownDigits[openPositions[std::rand() % openPositions.size()] * 2] = L'?'; }
                            }
                            for (int i = 0; i < 4; i++) {
                                if (secretCode.find(userInput[i]) == std::wstring::npos) {
                                    if (std::find(wrongDigits.begin(), wrongDigits.end(), userInput[i]) == wrongDigits.end()) { wrongDigits.push_back(userInput[i]); }
                                }
                            }
                            leaks.missingDigits = L"";
                            for (wchar_t c : wrongDigits) leaks.missingDigits += c + std::wstring(L", ");
                            if (!leaks.missingDigits.empty()) { leaks.missingDigits.pop_back(); leaks.missingDigits.pop_back(); }
                            userInput = L"";
                        }
                    }
                }
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (escapeBtnRect.getGlobalBounds().contains(mousePosF)) {
                        clickSound.play();
                        currentState = GameState::EscapeConfirm;
                        confirmBoxAlpha = 0.0f;
                    }
                }
            }
            else if (currentState == GameState::RoundStats) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                    clickSound.play();
                    totalWallet += earnedThisRound;

                    // !!! ПЕРЕРАБОТКА: Вместо моментальной победы на 3 раунде принудительно бросаем в ПОБЕГ !!!
                    if (currentRound >= TOTAL_ROUNDS) {
                        setupEmergencyConsole();
                    }
                    else {
                        currentRound++;
                        prepareRound();
                    }
                }
            }
            else if (currentState == GameState::EscapeConfirm) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Y) {
                        setupEmergencyConsole();
                    }
                    if (event.key.code == sf::Keyboard::N) {
                        clickSound.play();
                        currentState = GameState::Gameplay;
                    }
                }
            }
            else if (currentState == GameState::EmergencyConsole) {
                // !!! ЛЕГКИЙ ПОБЕГ: Разрешаем юзать X прямо в экстренной консоли !!!
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::X && skills.hasActivator) {
                    // Активатор в консоли побега просто генерирует новый легкий/простой код для текущей ячейки
                    clickSound.play();
                    secretCode = generateSecretCode(Difficulty::Easy);
                    std::wcout << L"[DEBUG ESCAPE BYPASS] Новый упрощенный код ячейки: " << secretCode << std::endl;
                    skills.hasActivator = false;
                }

                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode >= '0' && event.text.unicode <= '9' && userInput.length() < 4) {
                        clickSound.play();
                        userInput += static_cast<wchar_t>(event.text.unicode);
                    }
                    if (event.text.unicode == 8 && !userInput.empty()) {
                        clickSound.play();
                        userInput.pop_back();
                    }
                }
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                    if (userInput.length() == 4) {
                        if (userInput == secretCode) {
                            escapeCodesCracked++;
                            userInput = L"";
                            if (escapeCodesCracked >= escapeCodesNeeded) {
                                resetToNormalTheme();
                                currentState = GameState::Victory;
                            }
                            else {
                                secretCode = generateSecretCode(currentDiff);
                            }
                        }
                        else {
                            userInput = L"";
                        }
                    }
                }
            }
            else if (currentState == GameState::Victory || currentState == GameState::GameOver) {
                if (event.type == sf::Event::MouseButtonPressed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)) {
                    clickSound.play();
                    resetToNormalTheme();
                    currentState = GameState::MainMenu;
                }
            }
        }

        // --- ЛОГИКА ОБНОВЛЕНИЯ ДАННЫХ (UPDATE) ---
        float dt = gameClock.restart().asSeconds();

        menuAnimationTimer += dt * 3.0f;
        int dynamicAlpha = static_cast<int>(180 + 75 * std::sin(menuAnimationTimer));
        startBtn.setFillColor(sf::Color(0, dynamicAlpha, 0));
        diffBtn.setFillColor(sf::Color(0, dynamicAlpha, 0));
        loreBtn.setFillColor(sf::Color(0, dynamicAlpha, 0));

        escapeBlinkTimer += dt * 5.0f;
        int blinkValue = static_cast<int>(escapeBlinkTimer) % 2;
        if (blinkValue == 0) {
            escapeBtnRect.setOutlineColor(sf::Color(255, 0, 0, 255));
            escapeBtnText.setFillColor(sf::Color(255, 0, 0, 255));
        }
        else {
            escapeBtnRect.setOutlineColor(sf::Color(100, 0, 0, 255));
            escapeBtnText.setFillColor(sf::Color(100, 0, 0, 255));
        }

        if (currentState == GameState::EscapeConfirm) {
            confirmBoxAlpha += 400.0f * dt;
            if (confirmBoxAlpha > 255.0f) confirmBoxAlpha = 255.0f;
        }

        if (currentState == GameState::EmergencyConsole) {
            escapeTimer -= dt;
            escapeTypewriterTimer += dt * 300.0f;

            if (escapeTimer <= 0.0f) {
                escapeTimer = 0.0f;
                totalWallet *= 0.1f;
                resetToNormalTheme();
                currentState = GameState::GameOver;
            }
        }

        // --- ОТРИСОВКА КАДРА (RENDER) ---
        window.clear(sf::Color(2, 12, 2));

        if (hasNormalBg || hasEscapeBg) {
            window.draw(bgSprite);
        }

        if (currentState == GameState::MainMenu) {
            sf::RectangleShape menuBg(sf::Vector2f(1150, 600));
            menuBg.setFillColor(sf::Color(5, 10, 5, 210));
            menuBg.setOutlineColor(sf::Color(0, 200, 0, 100));
            menuBg.setOutlineThickness(2);
            menuBg.setPosition(350, 180);
            window.draw(menuBg);

            window.draw(menuTitle);
            window.draw(startBtn);
            window.draw(diffBtn);
            window.draw(loreBtn);
            window.draw(menuFooter);
        }

        else if (currentState == GameState::LoreInstructions) {
            sf::RectangleShape infoBg(sf::Vector2f(1600, 850));
            infoBg.setFillColor(sf::Color(5, 15, 25, 240));
            infoBg.setOutlineColor(sf::Color(0, 200, 255, 150));
            infoBg.setOutlineThickness(3);
            infoBg.setPosition(160, 100);
            window.draw(infoBg);

            loreContentText.setString(
                L" КТО ТЫ:  Ты — легендарный элитный нетраннер андеграунда.\n"
                L" ЦЕЛЬ:    Взломать 3 квантовых ядра. ВНИМАНИЕ: После взлома всех ядер активируется ПОБЕГ!\n"
                L" ОБЪЕКТ:  Секретная IT-лаборатория корпорации NEVADA LABORATORIES.\n"
                L" СИСТЕМА: Объект работает на экспериментальной квантовой архитектуре QUANTUM OS.\n\n"
                L" --------------------------------------------------------------------------\n"
                L"                      СПРАВОЧНИК АКТИВНОГО КИБЕР-СОФТА:\n"
                L" --------------------------------------------------------------------------\n"
                L"  [Z] ФАЕРВОЛЛ  - Блокирует один дебафф. СГОРАЕТ после использования!\n"
                L"  [X] АКТИВАТОР - Открывает символ за 20% куша. РАЗРЕШЕН И СГОРАЕТ В РЕЖИМЕ ПОБЕГА!\n"
                L"  [1] МАСКИРОВКА - Замедляет потерю кэша. СГОРАЕТ строго в конце раунда!\n\n"
                L"  >>> НАЖМИТЕ ЛЮБУЮ КЛАВИШУ ДЛЯ ВОЗВРАТА В ТЕРМИНАЛ МЕНЮ..."
            );

            window.draw(loreTitle);
            window.draw(loreContentText);
        }

        else if (currentState == GameState::DarkNetShop) {
            sf::RectangleShape shopBg(sf::Vector2f(1520, 850));
            shopBg.setFillColor(sf::Color(10, 5, 5, 230));
            shopBg.setOutlineColor(sf::Color(255, 30, 30, 100));
            shopBg.setOutlineThickness(2);
            shopBg.setPosition(200, 80);
            window.draw(shopBg);

            // Проверяем, активен ли уже в векторе модификатор маскировки
            bool hasMaskingPurchased = std::find(activeModifiers.begin(), activeModifiers.end(), Modifier::Masking) != activeModifiers.end();
            std::wstring mStat = hasMaskingPurchased ? L" [ИНТЕГРИРОВАНО]" : L"";
            std::wstring fStat = skills.hasCloudFirewall ? L" [ИНТЕГРИРОВАНО]" : L"";
            std::wstring aStat = skills.hasActivator ? L" [ИНТЕГРИРОВАНО]" : L"";

            shopWalletText.setString(L"БАНКОВСКИЙ КАНАЛ RE-ROUTE: " + std::to_wstring(static_cast<int>(totalWallet)) + L" USD\n"
                L"Нажимайте клавиши [1], [2], [3] для инжекции софта в хакерскую деку.");

            sItem1.setString(L"[ 1 ] КУПИТЬ МАСКИРОВКУ (Снижение убыли кэша на 1 раунд) - $600" + mStat);
            sItem2.setString(L"[ 2 ] КУПИТЬ АКТИВНЫЙ 'ОБЛАЧНЫЙ ФАЕРВОЛЛ [Z]' (Сброс дебаффа) - $600" + fStat);
            sItem3.setString(L"[ 3 ] КУПИТЬ АКТИВНЫЙ СКРИПТ 'СИСТЕМНЫЙ АКТИВАТОР [X]'  - $500" + aStat);

            if (hasMaskingPurchased) sItem1.setFillColor(sf::Color(0, 200, 0)); else sItem1.setFillColor(sf::Color::White);
            if (skills.hasCloudFirewall) sItem2.setFillColor(sf::Color(0, 200, 0)); else sItem2.setFillColor(sf::Color::White);
            if (skills.hasActivator) sItem3.setFillColor(sf::Color(0, 200, 0)); else sItem3.setFillColor(sf::Color::White);

            window.draw(shopBg);
            window.draw(shopTitle);
            window.draw(shopWalletText);
            window.draw(sItem1);
            window.draw(sItem2);
            window.draw(sItem3);
            window.draw(itemSkipBtn);
        }

        else if (currentState == GameState::IntroOverlay) {
            float elapsed = introClock.getElapsedTime().asSeconds();
            introOverlayText.setString(L"   ПОДКЛЮЧЕНИЕ HACKNEVADA... ИНЖЕКЦИЯ КУБИТОВ... УЗЕЛ СЕТИ " + std::to_wstring(currentRound) + L"\n" +
                L"   КВАНТОВЫЙ КУШ НА ХЕШ-АДРЕСЕ: 15000 USD\n\n   КВАНТОВАЯ НЕСТАБИЛЬНОСТЬ:\n   " + modAlertText);

            if (elapsed >= 2.0f) {
                introAlpha -= 180.0f * dt;
                if (introAlpha < 0) introAlpha = 0;
            }
            if (elapsed >= 3.2f) {
                currentState = GameState::Gameplay;
                autoLeakClock.restart();
            }

            introOverlayBox.setFillColor(sf::Color(10, 30, 15, static_cast<sf::Uint8>(introAlpha)));
            introOverlayText.setFillColor(sf::Color(0, 255, 0, static_cast<sf::Uint8>(introAlpha)));
            window.draw(introOverlayBox);
            window.draw(introOverlayText);
        }

        else if (currentState == GameState::Gameplay || currentState == GameState::EscapeConfirm) {
            if (isRoundWon) {
                winFreezeTimer += dt;
                hackCodeText.setFillColor(sf::Color(255, 255, 0));
                hackCodeText.setString(secretCode);

                if (winFreezeTimer >= 1.8f) {
                    currentState = GameState::RoundStats;
                }
            }
            else {
                currentRoundCash -= cashDrainSpeed * dt;
                if (currentRoundCash <= 0) { currentRoundCash = 0; resetToNormalTheme(); currentState = GameState::GameOver; }

                hackCodeText.setFillColor(sf::Color(255, 40, 40));

                if (autoLeakClock.getElapsedTime().asSeconds() >= leakInterval) {
                    autoLeakClock.restart();
                    std::vector<int> closedPositions;
                    for (int i = 0; i < 4; i++) { if (leaks.knownDigits[i * 2] == L'?') closedPositions.push_back(i); }
                    if (!closedPositions.empty()) { int rp = closedPositions[std::rand() % closedPositions.size()]; leaks.knownDigits[rp * 2] = secretCode[rp]; }
                }

                std::wstring displayCode = L"&&&&";
                for (int i = 0; i < 4; i++) {
                    if (leaks.knownDigits[i * 2] != L'?') displayCode[i] = leaks.knownDigits[i * 2];
                    else displayCode[i] = noiseSymbols[std::rand() % noiseSymbols.length()];
                }
                hackCodeText.setString(displayCode);
            }

            displayedCash -= (displayedCash - currentRoundCash) * 10.0f * dt;

            std::wstring statusStr =
                L"=========================================\n"
                L"       NEVADA LABS TERMINAL COMPILER     \n"
                L"=========================================\n\n"
                L" КВАНТОВОЕ ЯДРО:    " + std::to_wstring(currentRound) + L" / " + std::to_wstring(TOTAL_ROUNDS) + L"\n" +
                L" СТАБИЛЬНОСТЬ КУША: " + std::to_wstring(static_cast<int>(displayedCash)) + L" USD ↓\n" +
                L" ЧИСТЫЙ БАЛАНС:     " + std::to_wstring(static_cast<int>(totalWallet)) + L" USD\n\n" +
                L" МОДИФИКАТОРЫ QUANTUM OS: \n " + modAlertText + L"\n\n" +
                L" ПОТОК ВВОДА:       " + userInput + L"_\n" +
                L" ОШИБКИ АНАЛИЗА:    " + std::to_wstring(wrongAttemptsCount) + L"\n\n"
                L"-----------------------------------------\n" +
                L"         МАТРИЦА УТЕЧКИ СУБСТРУКТУР      \n" +
                L"-----------------------------------------\n" +
                L" [АНАЛИЗАТОР КЛЮЧА]: " + leaks.knownDigits + L"\n" +
                L" [КВАНТОВАЯ СУММА]: " + std::to_wstring(leaks.sumOfDigits) + L"\n" +
                L" [КВАНТОВЫЙ МУСОР]: " + leaks.missingDigits + L"\n"
                L"=========================================";

            if (!isRoundWon) {
                typewriterTimer += dt * 350.0f;
                typewriterLength = static_cast<size_t>(typewriterTimer);
                if (typewriterLength > statusStr.length()) typewriterLength = statusStr.length();
                leftLogText.setString(statusStr.substr(0, typewriterLength));
            }
            else {
                leftLogText.setString(statusStr);
            }

            std::wstring skillPanel = L"--- КИБЕРНЕТИЧЕСКАЯ ДЕКА: АКТИВНЫЙ СОФТ ---\n";
            if (skills.hasCloudFirewall) {
                skillPanel += L" [Z] ОБЛАЧНЫЙ ФАЕРВОЛЛ:: ГОТОВ К ЗАПУСКУ\n";
            }
            else skillPanel += L" [Z] ОБЛАЧНЫЙ ФАЕРВОЛЛ:: НЕ КУПЛЕНО / СГОРЕЛ\n";

            if (skills.hasActivator) {
                skillPanel += L" [X] АКТИВАТОР ЯДРА   :: ДОСТУПЕН (Сгорит при вводе)\n";
            }
            else skillPanel += L" [X] АКТИВАТОР ЯДРА   :: НЕ КУПЛЕНО / СГОРЕЛ\n";

            activeSkillsText.setString(skillPanel);

            window.draw(leftLogRect);
            window.draw(hackWindowRect);

            window.draw(leftLogText);
            window.draw(hackWindowTitle);
            window.draw(hackCodeText);
            window.draw(activeSkillsText);

            window.draw(escapeBtnRect);
            window.draw(escapeBtnText);

            if (currentState == GameState::EscapeConfirm) {
                confirmBox.setOutlineColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(confirmBoxAlpha)));
                confirmBox.setFillColor(sf::Color(20, 5, 5, static_cast<sf::Uint8>(confirmBoxAlpha)));
                confirmText.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(confirmBoxAlpha)));

                window.draw(confirmBox);
                window.draw(confirmText);
            }
        }

        else if (currentState == GameState::RoundStats) {
            std::wstring endMessage = (currentRound >= TOTAL_ROUNDS) ?
                L" >>> ВНИМАНИЕ: ВСЕ ЯДРА СЛОМАНЫ! НАЖМИТЕ [SPACE] ДЛЯ ИНИЦИАЛИЗАЦИИ ПОБЕГА..." :
                L" >>> НАЖМИТЕ [SPACE] ДЛЯ ПОДКЛЮЧЕНИЯ К СЛЕДУЮЩЕМУ УЗЛУ...";

            std::wstring statsStr =
                L" СТАТУС НОДЫ:       УСПЕШНО ДЕШИФРОВАНО [OK]\n"
                L" КВАНТОВЫЙ КЛЮЧ:    " + secretCode + L"\n"
                L" ЗАФИКСИРОВАНО ЦИКЛОВ: " + std::to_wstring(wrongAttemptsCount) + L" (КОРРЕКТИРОВКИ ОШИБОК)\n\n"
                L"----------------------------------------------------\n"
                L" ФИНАНСОВЫЙ КАНАЛ RE-ROUTE ПРОВЕДЕН:\n"
                L" ПОЛУЧЕНО НА БАЛАНС: +" + std::to_wstring(static_cast<int>(earnedThisRound)) + L" USD\n"
                L" ТЕКУЩИЙ ОБЩИЙ СЧЕТ:  " + std::to_wstring(static_cast<int>(totalWallet + earnedThisRound)) + L" USD\n"
                L"----------------------------------------------------\n\n" + endMessage;

            statsContentText.setString(statsStr);

            window.draw(statsBoxRect);
            window.draw(statsTitleText);
            window.draw(statsContentText);
        }

        else if (currentState == GameState::EmergencyConsole) {
            sf::RectangleShape consoleBg(sf::Vector2f(1760, 920));
            consoleBg.setFillColor(sf::Color(2, 8, 2, 240));
            consoleBg.setOutlineColor(sf::Color(0, 180, 0));
            consoleBg.setOutlineThickness(2);
            consoleBg.setPosition(80, 50);
            window.draw(consoleBg);

            int minutes = static_cast<int>(escapeTimer) / 60;
            int seconds = static_cast<int>(escapeTimer) % 60;
            std::wstring secondsStr = (seconds < 10) ? L"0" + std::to_wstring(seconds) : std::to_wstring(seconds);

            std::wstring actStatus = skills.hasActivator ? L"ДОСТУПЕН [Нажмите X для обхода ядра]" : L"НЕ КУПЛЕН / ИСПОЛЬЗОВАН";

            std::wstring fullConsoleStr =
                L"====================================================================\n"
                L"   CRITICAL EMERGENCY TERMINAL: QUANTUM COMPILER OVERRIDE           \n"
                L"====================================================================\n\n"
                L" СТАТУС OS: АВАРИЙНАЯ БЛОКИРОВКА СЕТИ. СБРОС ИНТЕРФЕЙСА QUANTUM OS.\n"
                L" АКТИВИРОВАН ПРЯМОЙ ОБХОД МЕЙНФРЕЙМА С ПОМОЩЬЮ HACKNEVADA ROOT КИТА.\n"
                L" [ДОСТУПНЫЙ СОФТ ПОБЕГА]: АКТИВАТОР ДЕКИ [X] - " + actStatus + L"\n\n"
                L" СЧЕТЧИК КВАНТОВОГО СХЛОПЫВАНИЯ:   " + std::to_wstring(minutes) + L":" + secondsStr + L" МИН. !!!\n"
                L" СТАБИЛИЗИРОВАНО МАТРИЦ ДОСТУПА:    " + std::to_wstring(escapeCodesCracked) + L" / " + std::to_wstring(escapeCodesNeeded) + L"\n\n"
                L" ЗАФИКСИРОВАННЫЙ ОБЪЕМ ДАННЫХ:      " + std::to_wstring(static_cast<int>(totalWallet)) + L" USD\n\n"
                L"--------------------------------------------------------------------\n"
                L" ЗАПРОС ПРЯМОГО ХЕША КВАНТОВОЙ ЯЧЕЙКИ # " + std::to_wstring(escapeCodesCracked + 1) + L"\n"
                L" СИСТЕМА ОЖИДАЕТ ВВОД ДЕШИФРАТОРА: " + userInput + L"_\n"
                L"--------------------------------------------------------------------\n\n"
                L" >>> НАЖМИТЕ ENTER ДЛЯ ФОРСИРОВАНИЯ КВАНТОВОГО ПАКЕТА...";

            size_t consoleLength = static_cast<size_t>(escapeTypewriterTimer);
            if (consoleLength > fullConsoleStr.length()) {
                consoleLength = fullConsoleStr.length();
            }

            consoleText.setString(fullConsoleStr.substr(0, consoleLength));
            window.draw(consoleText);
        }

        else if (currentState == GameState::Victory) {
            sf::RectangleShape endBg(sf::Vector2f(1300, 400));
            endBg.setFillColor(sf::Color(5, 20, 5, 240));
            endBg.setPosition(310, 340);
            window.draw(endBg);

            mainText.setString(L"                 [ КВАНТОВЫЙ ВЗЛОМ ЗАВЕРШЕН ]\n\n"
                L"СИСТЕМЫ QUANTUM OS РАЗОРВАНЫ. HACKNEVADA УСПЕШНО ОЧИСТИЛ СЛЕДЫ.\n"
                L"ВАШ ИТОГОВЫЙ ЗАРАБОТОК (ЧИСТЫЙ КУШ): " + std::to_wstring(static_cast<int>(totalWallet)) + L" USD\n\n"
                L"[ НАЖМИТЕ ПРОБЕЛ ДЛЯ ВЫХОДА В МЕНЮ... ]");
            window.draw(mainText);
        }

        else if (currentState == GameState::GameOver) {
            sf::RectangleShape endBg(sf::Vector2f(1300, 400));
            endBg.setFillColor(sf::Color(20, 5, 5, 240));
            endBg.setPosition(310, 340);
            window.draw(endBg);

            mainText.setString(L"                 [ ПОЛНОЕ ЗАТЕМНЕНИЕ СЕТИ ]\n\n"
                L"СИСТЕМА QUANTUM OS СХЛОПНУЛАСЬ. ХЕШ-АДРЕСА СМЕНИЛИСЬ, НЕВАДА ЗАБЛОКИРОВАНА.\n"
                L"ВЫ СУМЕЛИ ВЫТАЩИТЬ ВСЕГО: " + std::to_wstring(static_cast<int>(totalWallet)) + L" USD\n\n"
                L"[ НАЖМИТЕ ПРОБЕЛ ДЛЯ ВЫХОДА В МЕНЮ... ]");
            window.draw(mainText);
        }

        window.display();
    }

    return 0;
}