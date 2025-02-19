# Embarcatech ADC <https://youtu.be/gEsZm0hUAzgv>

Este projeto demonstra o uso do Raspberry Pi Pico para:
- Controlar um display SSD1306 via I2C.
- Ler valores de um joystick através do ADC.
- Gerar PWM para controlar LEDs.
- Utilizar interrupções para tratar botões (ex.: o botão do joystick alterna o LED verde e ativa/desativa uma borda no display).

## Funcionalidades

- **Display SSD1306:**  
  Inicializa e configura o display via I2C e desenha elementos gráficos (um quadrado e, opcionalmente, uma borda).

- **Joystick:**  
  Lê os eixos X e Y usando ADC para mover um quadrado no display.

- **PWM para LEDs:**  
  Controla os LEDs vermelho e azul via PWM, ajustando o brilho conforme a posição do joystick.

- **Interrupções de Botões:**  
  - **Joystick Button (GPIO 22):** Ao ser pressionado, alterna o estado do LED verde e a exibição de uma borda no display.
  - **BTN_A (GPIO 5):** Alterna o estado do PWM para os LEDs vermelho e azul.

## Requisitos de Hardware

- **Raspberry Pi Pico** (ou outra placa com RP2040)
- **Display SSD1306** (I2C)
- **Joystick:**  
  - Saídas analógicas para X e Y  
  - Botão digital
- **LEDs:**  
  - LED Vermelho (GPIO 13)  
  - LED Azul (GPIO 12)  
  - LED Verde (GPIO 11)
- Protoboard, jumpers e resistores conforme necessário.

### Exemplo de Conexões

- **Display SSD1306:**  
  - SDA: GPIO 14  
  - SCL: GPIO 15
- **Joystick:**  
  - Eixo X: ADC (GPIO 26)  
  - Eixo Y: ADC (GPIO 27)  
  - Botão: GPIO 22
- **BTN_A:**  
  - Conectado ao GPIO 5
- **LEDs:**  
  - LED_RED: GPIO 13  
  - LED_BLUE: GPIO 12  
  - LED_GREEN: GPIO 11

## Configuração do Projeto no VS Code com o Pico SDK Plugin

1. **Instalação do VS Code e Plugin do Pico SDK:**
   - Instale o [Visual Studio Code](https://code.visualstudio.com/).
   - No VS Code, instale a extensão [Pico SDK](https://marketplace.visualstudio.com/items?itemName=klauer.pico-sdk) (ou a extensão recomendada para o desenvolvimento com RP2040).

2. **Clonando o Repositório:**
   ```bash
   git clone https://github.com/Samir21203/embarcatech_adc.git
   cd embarcatech_adc
   ```

3. **Abrindo o Projeto:**
   - Abra a pasta do projeto no VS Code.
   - O plugin do Pico SDK deverá detectar automaticamente o `CMakeLists.txt` e sugerir a configuração do ambiente.

4. **Configuração do Ambiente:**
   - Caso não esteja configurado, defina a variável de ambiente `PICO_SDK_PATH` apontando para o diretório do Pico SDK.
   - No VS Code, abra a paleta de comandos (`Ctrl+Shift+P`) e selecione:
     - `Pico SDK: Configure project`  
     - `CMake: Configure` para gerar os arquivos de build.

5. **Compilação:**
   - Utilize o comando `CMake: Build` (via paleta de comandos ou pela barra de status) para compilar o projeto.
   - O VS Code e o plugin utilizarão o Ninja (ou a ferramenta configurada) para gerar o firmware.

6. **Flash do Firmware:**
   - Conecte o Raspberry Pi Pico no modo BOOTSEL.
   - No VS Code, utilize o comando `Pico SDK: Flash` (ou copie manualmente o arquivo `.uf2` gerado para a unidade USB do Pico).

## Estrutura do Projeto

```
embarcatech_adc/
├── CMakeLists.txt        # Configuração do CMake para o projeto
├── build/                # Diretório gerado com os arquivos de build (não versionar)
├── include/
│   └── ssd1306.h         # Cabeçalho da biblioteca SSD1306
├── ssd1306.c             # Implementação da biblioteca SSD1306
└── embarcatech_adc.c     # Código-fonte principal do projeto
```

## Funcionamento

- **Inicialização:**  
  O display, os ADCs e os pinos dos LEDs são configurados. As interrupções são registradas para os botões (o botão do joystick e o BTN_A).

- **Loop Principal:**  
  - São lidos os valores dos eixos do joystick.
  - É calculado o deslocamento de um quadrado no display conforme a posição do joystick.
  - Os níveis de PWM para os LEDs vermelho e azul são ajustados.
  - O display é atualizado com o desenho do quadrado e, se ativado, com uma borda.

- **Interrupção Global:**  
  Um callback único trata as interrupções dos botões:
  - **Joystick Button (GPIO 22):**  
    Na borda de descida, alterna o estado do LED verde e a exibição da borda.
  - **BTN_A (GPIO 5):**  
    Alterna o estado do PWM dos LEDs vermelho e azul.

## Debug

- Utilize o terminal serial (UART) para visualizar mensagens de debug, se implementadas.
- Se o botão do joystick não funcionar como esperado, verifique as conexões físicas (o botão deve efetivamente conectar o pino ao GND quando pressionado).

## Créditos

Desenvolvido por Victor Samir – 2025.
