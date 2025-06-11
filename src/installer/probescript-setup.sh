#!/bin/bash

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

    if [ -f "/usr/local/bin/probescript" ]; then
        echo "Probescript is already installed in /usr/local/bin/"
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
    echo "Default installation directory: /usr/local/bin"
    read -p "Enter installation directory (or press Enter for default): " install_dir
    
    if [ -z "$install_dir" ]; then
        install_dir="/usr/local/bin"
    fi

    echo
    echo "Installing probescript to $install_dir/"
    
    if [ ! -d "$install_dir" ]; then
        echo "Creating directory: $install_dir"
        sudo mkdir -p "$install_dir"
        if [ $? -ne 0 ]; then
            echo "Error: Could not create installation directory."
            echo "This may be due to insufficient privileges."
            read -p "Press Enter to continue..."
            main_menu
            return
        fi
    fi

    sudo cp "$binary_path" "$install_dir/probescript"
    if [ $? -ne 0 ]; then
        echo "Error: Could not copy file."
        read -p "Press Enter to continue..."
        main_menu
        return
    fi

    sudo chmod +x "$install_dir/probescript"
    if [ $? -ne 0 ]; then
        echo "Warning: Could not set executable permissions."
    fi

    echo "Probescript installed to: $install_dir/probescript"

    if [ "$install_dir" = "/usr/local/bin" ]; then
        echo
        echo "Installation completed!"
        show_ascii_art
        echo
        echo "Welcome to probescript!"
        echo "To see a list of commands, run: probescript --help"
        echo "Repository: https://github.com/slurpy-films/probescript"
    else
        echo
        read -p "Add $install_dir to PATH? (y/n): " add_path
        if [[ "$add_path" =~ ^[Yy]$ ]]; then
            add_to_path "$install_dir"
        fi
        
        echo
        echo "Installation completed!"
        show_ascii_art
        echo
        echo "Welcome to probescript!"
        echo "To see a list of commands, run: probescript --help"
        echo "Repository: https://github.com/slurpy-films/probescript"
    fi

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

    if [ ! -f "/usr/local/bin/probescript" ]; then
        echo "Probescript is not installed in standard location."
        echo "Checking other possible locations..."
        
        if command -v probescript &> /dev/null; then
            probescript_location=$(which probescript)
            echo "Probescript found at: $probescript_location"
            echo "You may need to manually remove it."
        else
            echo "Probescript was not found on the system."
        fi
        
        read -p "Press Enter to continue..."
        main_menu
        return
    fi

    echo "Probescript found at: /usr/local/bin/probescript"
    echo
    read -p "Are you sure you want to uninstall probescript? (y/n): " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        main_menu
        return
    fi

    echo "Removing probescript..."
    sudo rm -f "/usr/local/bin/probescript"
    
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

    if [ -f "/usr/local/bin/probescript" ]; then
        echo "[OK] Probescript found at: /usr/local/bin/probescript"
        
        file_info=$(ls -lh /usr/local/bin/probescript)
        echo "     File info: $file_info"
    else
        echo "[NOT FOUND] Probescript not found in standard location"
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
    echo "Adding $new_path to PATH..."

    shell_config=""
    if [ -n "$BASH_VERSION" ]; then
        if [ -f "$HOME/.bashrc" ]; then
            shell_config="$HOME/.bashrc"
        elif [ -f "$HOME/.bash_profile" ]; then
            shell_config="$HOME/.bash_profile"
        fi
    elif [ -n "$ZSH_VERSION" ]; then
        shell_config="$HOME/.zshrc"
    fi

    if [ -z "$shell_config" ]; then
        echo "Could not determine shell configuration file."
        echo "Please manually add $new_path to your PATH."
        return
    fi

    if grep -q "$new_path" "$shell_config" 2>/dev/null; then
        echo "PATH already contains probescript location."
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