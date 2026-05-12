# Asteroids GBA

Um port da mecânica clássica de Asteroids desenvolvido em C (bare-metal) para o hardware do Game Boy Advance.

## 🛠️ Pré-requisitos

Para compilar e testar este projeto, você precisará de:
* **devkitPro** (com a toolchain `devkitARM`)
* **mGBA** (ou outro emulador de GBA da sua preferência)

## 📦 Instalação do devkitPro (Linux)

O devkitPro distribui suas ferramentas através do gerenciador de pacotes `pacman`.

1. **Configure o repositório:** Dependendo da sua distribuição, instale a ferramenta `dkp-pacman` fornecida pelo devkitPro ou adicione o repositório deles ao seu `pacman.conf` nativo. (Consulte a [wiki oficial do devkitPro](https://devkitpro.org/wiki/Getting_Started) para o script exato da sua distro).

2. **Instale o pacote do GBA:** Com o pacman configurado, instale as bibliotecas e o compilador ARM:
   ```bash
   sudo pacman -S gba-dev
   # ou dkp-pacman -S gba-dev
   ```

3. **Configure as variáveis de ambiente:** Adicione estas linhas ao final do seu arquivo de configuração do terminal (`~/.bashrc` ou `~/.zshrc`) para que o `make` saiba onde encontrar o compilador:
   ```bash
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=$DEVKITPRO/devkitARM
   ```
   *Reinicie o terminal após adicionar as linhas.*

## 🚀 Como Compilar

1. Abra o terminal na raiz deste projeto (na mesma pasta onde está o arquivo `Makefile`).
2. Para compilar o jogo, simplesmente rode:
   ```bash
   make
   ```
   *(Nota: Se precisar forçar uma recompilação do zero, como após alterar gráficos, use `make clean && make`).*

3. O compilador irá gerar um arquivo com a extensão `.gba` (ex: `asteroids-gba.gba`).

## 🎮 Como Jogar

Abra o arquivo `.gba` gerado no seu emulador. Via terminal, você pode rodar direto com:
   ```bash
   mgba-qt asteroids.gba
   ```

**Controles:**
* **Esquerda / Direita:** Rotaciona a nave.
* **Cima:** Acelera (Injeção de inércia).
* **Baixo:** Freio (Amortecedor inercial).
* **Botão A:** Atira.

# References

* https://www.reddit.com/r/PixelArt/comments/jy89x7/new_spaceship_that_i_made_i_am_still_quite_new_to/
* https://deep-fold.itch.io/space-background-generator