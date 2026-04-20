#!/bin/bash

# Define color codes
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
NC='\033[0m'

# Log function with color
log() {
    local color=$1
    local text=$2
    echo -e "${color}${text}${NC}"
}

# Error handling function
handle_error() {
    log $RED "ERROR: $1"
    exit 1
}

getconfig() {
    local section=$1
    local key=$2
    local file='build_config.ini'
    local cursec=""
    local result=""


    if [ ! -f "$file" ]; then
       handle_error "Error: Configuration file $file not found!"
    fi

    awk '{print}' "$file" > temp_file && mv temp_file "$file"
   
    while IFS= read -r line; do
     
        trimmed_line=$(echo "$line" | tr -d '\r' | xargs)
  
        case "$trimmed_line" in
            ''|'#'* )
                continue
                ;;
        esac

        if echo "$trimmed_line" | grep -q '^\['; then
            cursec=$(echo "$trimmed_line" | cut -d']' -f1 | cut -d'[' -f2)
            continue
        fi

        if [ "$cursec" = "$section" ]; then
            if echo "$trimmed_line" | grep -q '='; then
                conf_key=$(echo "$trimmed_line" | cut -d'=' -f1 | xargs)  
                conf_value=$(echo "$trimmed_line" | cut -d'=' -f2- | xargs)

                if [ "$conf_key" = "$key" ]; then
                    result="$conf_value"
                    break
                fi
            fi
        fi
    done < "$file"

    echo "$result"
}

help() {
    local helpfile='help.txt'
    if [ ! -f $helpfile ]; then
        handle_error "Help file '$helpfile' not found!"
    fi

    awk '{print}' " $helpfile" > temp_file && mv temp_file " $helpfile"

    cat $helpfile | while IFS= read -r line; do
        log "$GREEN" "$line"
    done
}

run_make() {
    local target=$1
    make -f Makefile KERDIR="$KDIR" CROSS_COMPILE="$CC" CROSS_COMPILE_BLE_STACK="$CCBLE" arch="$architecture" sys="$system" platform="$platform_soc" wifi_platform="$wifi_platform" "$target"
}


run_compile() {
    local platform=$1
    local action=${2:-"default"}

    log $GREEN "\n\nStarting compilation for platform: $platform with action: $action\n"


    # Retrieve configuration values
    retrieve_config() {
        local key=$1
        local value=$(getconfig "$platform" "$key")
       if [ -z "$value" ]; then
            handle_error "Failed to get $key for platform $platform"
        else
            #log $GREEN "$key set to: $value"
            eval "$key=\"$value\""
        fi
    }

    retrieve_config KDIR
    retrieve_config CC
    retrieve_config architecture
    retrieve_config system
    retrieve_config platform_soc
    retrieve_config wifi_platform

    LD_LIBRARY_PATH=$(getconfig "$platform" "LD_LIBRARY_PATH")
    if [ ! -z "$LD_LIBRARY_PATH" ];  then
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
        log $GREEN "LD_LIBRARY_PATH set to: $LD_LIBRARY_PATH"
    fi

    CCBLE=$(getconfig "$platform" "CCBLE")
    if [ ! -z "$CCBLE" ];  then
        log $GREEN "CCBLE set to: $CCBLE"
    else
        CCBLE=$CC
        log $GREEN "CCBLE set to: $CCBLE"
    fi

   # Execute compile actions based on provided action
    case "$action" in
        "ble_stack")
            log $GREEN "Compiling ble_stack..."
            run_make "ble_stack"
            ;;
        "clean")
            log $GREEN "clean dirver and ble stack..."
            run_make "clean"
            run_make "ble_stack_clean"
            ;;
        "conf")
            log $GREEN "menu configuration"
            run_make "menuconfig"
            ;;
        "driver")
            log $GREEN "Compiling driver..."
            run_make "clean"
            run_make all
            run_make "strip"
            ;;
        "default")
            log $GREEN "Compiling all..."
            run_make "clean"
            run_make "ble_stack_clean"
            run_make all
            run_make "strip"
            run_make "ble_stack"
            ;;
        *)
            log $GREEN "Unknown action: $action"
            help
            return 1
            ;;
    esac

    log $GREEN "\n\nCompilation completed for platform: $platform with action: $action\n"
}

main() {

    local platform=$1
    local action=$2

    case "$platform" in
        "help")
            help
            ;;
        "clean"| "conf"| "driver")
            handle_error "No SOC specified."
            ;;
        *)
            if [ -z "$platform" ]; then
                help
                handle_error "Error: Both action and SOC option are required. Please specify both."
            else
                run_compile "$platform" "$action"
            fi
            ;;
    esac

}

main "$@"