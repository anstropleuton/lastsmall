# Optional, just to align stuff
start:
    # Define variables
    int a
    int b

    # Text to be printed
    string prompt = "Enter a value: "

    # Ask user for input
    print prompt
    scan a
    print prompt
    scan b

    # Set up call parameters
    int add_parameter_1 = a # Or can evaluate only a single variable
    int add_parameter_2 = b

    # Call the function
    # This will set up a point at which the return will come back
    call add

    # We get a return value from call
    int c = add_return_value

    # Delete unneeded stuff
    delete add_parameter_1
    delete add_parameter_2
    delete add_return_value

    # Print a formatted text
    # Preparation
    string statement_1 = "The sum of "
    string statement_2 = " and "
    string statement_3 = " is "
    string statement_4 = "\n"

    # Actually print
    print statement_1
    print a
    print statement_2
    print b
    print statement_3
    print c
    print statement_4

    # Use exit to end the program, or it will fall through the add:
    # This also deletes the variables and goes to next file
    # If you want to keep the variables, just make a new label at the end of this file and goto it
    exit

# Add function, don't do `goto add`, but do `call add` because this is how it is set up
add:
    # Ensure return value is deleted to be newly created
    exists add_return_value add_return_value_exists
    goto add_return_value_not_exists

add_return_value_exists:
    delete add_return_value

add_return_value_not_exists:
    # Ensure parameters exist
    exists add_parameter_1 add_parameter_1_exists
    int add_return_value = 0
    return

add_parameter_1_exists:
    exists add_parameter_2 add_parameter_2_exists
    int add_return_value = 0
    return

# Actual add function
add_parameter_2_exists:
    int add_return_value # Cannot evaluate expression on declaration
    # So split the line
    add_return_value = add_parameter_1 + add_parameter_2
    return
