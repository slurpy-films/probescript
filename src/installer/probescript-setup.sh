#!/bin/bash

INSTALL_DIR="$HOME/.local/bin"

main_menu() {
    clear
    echo
    echo "========================================"
    echo "          PROBESCRIPT SETUP"
    echo "========================================"
    echo
    echo "Select an option:"
    echo
    echo "[1] Install probescript"
    echo "[2] Uninstall probescript"
    echo "[3] Check installation status"
    echo "[4] Exit"
    echo
    read -p "Enter your choice (1-4): " choice

    case $choice in
        1) install_probescript ;;
        2) uninstall_probescript ;;
        3) check_status ;;
        4) exit_program ;;
        *) echo "Invalid choice. Try again."; read -p "Press Enter to continue..."; main_menu ;;
    esac
}

install_probescript() {
    clear
    echo
    echo "========================================"
    echo "          INSTALL PROBESCRIPT"
    echo "========================================"
    echo

    mkdir -p "$INSTALL_DIR"

    if [ -f "$INSTALL_DIR/probescript" ]; then
        echo "Probescript is already installed in $INSTALL_DIR/"
        echo
        read -p "Do you want to reinstall? (y/n): " reinstall
        if [[ ! "$reinstall" =~ ^[Yy]$ ]]; then
            main_menu
            return
        fi
    fi

    local_binary="./probescript"
    if [ -f "$local_binary" ]; then
        echo "Found probescript in current directory."
        binary_path="$local_binary"
    else
        echo "Enter path to probescript binary:"
        read -p "Path (or press Enter to cancel): " binary_path

        if [ -z "$binary_path" ]; then
            echo "No path provided. Cancelling installation."
            read -p "Press Enter to continue..."
            main_menu
            return
        fi

        if [ ! -f "$binary_path" ]; then
            echo "Error: File does not exist."
            read -p "Press Enter to continue..."
            main_menu
            return
        fi
    fi

    echo
    echo "Installing probescript to $INSTALL_DIR/"
    cp "$binary_path" "$INSTALL_DIR/probescript"
    chmod +x "$INSTALL_DIR/probescript"

    echo
    if ! echo "$PATH" | grep -q "$INSTALL_DIR"; then
        add_to_path "$INSTALL_DIR"
    fi

    echo
    echo "Installation completed!"
    show_ascii_art
    echo
    echo "Welcome to probescript!"
    echo "To see a list of commands, run: probescript --help"
    echo "Repository: https://github.com/slurpy-films/probescript"
    echo
    read -p "Press Enter to continue..."
    main_menu
}

uninstall_probescript() {
    clear
    echo
    echo "========================================"
    echo "         UNINSTALL PROBESCRIPT"
    echo "========================================"
    echo

    if [ ! -f "$INSTALL_DIR/probescript" ]; then
        echo "Probescript is not installed in $INSTALL_DIR"
        read -p "Press Enter to continue..."
        main_menu
        return
    fi

    echo "Probescript found at: $INSTALL_DIR/probescript"
    echo
    read -p "Are you sure you want to uninstall probescript? (y/n): " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        main_menu
        return
    fi

    rm -f "$INSTALL_DIR/probescript"

    if [ $? -eq 0 ]; then
        echo "Probescript has been uninstalled."
    else
        echo "Error: Could not remove probescript."
    fi

    echo
    echo "Uninstallation completed!"
    read -p "Press Enter to continue..."
    main_menu
}

check_status() {
    clear
    echo
    echo "========================================"
    echo "      PROBESCRIPT INSTALLATION STATUS"
    echo "========================================"
    echo

    if [ -f "$INSTALL_DIR/probescript" ]; then
        echo "[OK] Probescript found at: $INSTALL_DIR/probescript"
        file_info=$(ls -lh "$INSTALL_DIR/probescript")
        echo "     File info: $file_info"
    else
        echo "[NOT FOUND] Probescript not found in $INSTALL_DIR"
    fi

    echo
    echo "Checking PATH..."
    if command -v probescript &> /dev/null; then
        probescript_location=$(which probescript)
        echo "[OK] Probescript found in PATH: $probescript_location"
    else
        echo "[NOT IN PATH] Probescript not found in PATH"
    fi

    echo
    echo "Testing probescript..."
    if command -v probescript &> /dev/null; then
        if probescript --version &> /dev/null; then
            echo "[OK] Probescript runs as expected"
            probescript --version 2>/dev/null || echo "Version info not available"
        else
            echo "[ERROR] Probescript found but could not run"
        fi
    else
        echo "[ERROR] Could not run probescript"
    fi

    echo
    read -p "Press Enter to continue..."
    main_menu
}

add_to_path() {
    local new_path="$1"
    echo
    echo "Adding $new_path to PATH..."

    local shell_config=""
    if [ -n "$BASH_VERSION" ]; then
        shell_config="$HOME/.bashrc"
    elif [ -n "$ZSH_VERSION" ]; then
        shell_config="$HOME/.zshrc"
    fi

    if [ -z "$shell_config" ]; then
        echo "Could not detect shell config. Please add $new_path to your PATH manually."
        return
    fi

    if grep -q "$new_path" "$shell_config"; then
        echo "PATH already contains $new_path"
        return
    fi

    echo "export PATH=\"$new_path:\$PATH\"" >> "$shell_config"
    echo "Added to $shell_config"
    echo "Please restart your terminal or run: source $shell_config"
}

show_ascii_art() {
    echo
    echo "                 ________  ________  ________  ________  _______   ________  ________  ________  ___  ________  _________   "
    echo "                |\\   __  \\|\\   __  \\ \\   __  \\|\\   __  \\|\\  ___ \\ |\\   ____|\\   ____|\\   __  \\|\\  \\|\\   __  \\|\\___   ___\\"
    echo "                \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\|\\ /\\ \\   __/ \\ \\  \\___|\\  \\___|\\  \\|\\  \\ \\  \\ \\  \\|\\  \\|___ \\  \\_| "
    echo "                 \\ \\   ____\\ \\   _  _\\ \\  \\\\\\  \\ \\   __  \\ \\  \\   _\\ \\_____  \\ \\  \\    \\ \\   _  _\\ \\  \\ \\   ____\\   \\ \\  \\"
    echo "                  \\ \\  \\___|\\  \\\\  \\\\ \\  \\\\\\  \\ \\  \\|\\  \\ \\  \\_|\\ \\|____|\\  \\ \\  \\____\\ \\  \\\\  \\\\ \\  \\ \\  \\___|    \\ \\  \\"
    echo "                   \\ \\__\\    \\ \\__\\\\ _\\\\ \\_______\\ \\_______\\ \\_______\\____\\_\\  \\ \\_______\\ \\__\\\\ _\\\\ \\__\\ \\__\\        \\ \\__\\"
    echo "                    \\|__|     \\|__|\\|__|\\|_______|\\|_______|\\|_______|\\________\\|_______|\\|__|\\|__|\\|__|\\|__|         \\|__|"
    echo "                                                                     \\|_______|                                                                                                    "
}

exit_program() {
    exit 0
}

main_menu