# STM32L432KC Clock Configuration Tutorial

This tutorial includes code to configure the PLL on the STM32L432KC.
It is a structured tutorial Git repository where the commits are designed to represent different steps in the configuration process.
A step-by-step solution is present on the `solution` branch and can be checked out at any time.
The `dev` branch is where you should do your development work.
You can compare your work at any time to the solution code.

To complete the tutorial you should add commits on the `dev` branch that properly configure the clock to run at the maximum speed of 84 MHz using the phase-locked loop (PLL) peripheral.

# Steps

This tutorial contains partially completed code snippets to configure the PLL.
The basic procedure for changing the clock speed is as follows:

1. Adjust the number of waitstates for the flash as required (see reference manual for details on what the latency needs to be set to for a given clock speed and core voltage).
2. Configure the PLL to set the desired clock frequency by configuring the appropriate values for:
   * The input clock source
   * The dividers (M, R) and multipliers (N)
3. Enable the PLL output
4. Once the PLL is locked, swap the system clock source to the PLL.

You need to choose the proper values for the PLL configuration in the `configurePLL()` function in `STM32L432KC_RCC.h`.

(Note that on the STM32L432KC, the clock speed should not exceed 80 MHz).

# Tips

This tutorial is structured in git to allow you to do some interesting things. 
For example, you can compare your current changes on the `dev` branch with the final solution to look at line-by-line changes using `git diff`.

There are many ways to use `git diff` but here is one to try.

## Compare your current branch with the tip of another one

`git diff <current branch> <solution branch> <file to diff>`

For example, run the following to see your changes compared to the final state of the solution file.

`git diff dev solution lib/STM32L432KC_RCC.c`

## Compare your current branch with a commit in the revision history of another branch

You can also easily refer to commits that are further back in the history by using the tilde.
For example, if you want to compare to the commit that is two commits back from the tip of the `solution` branch, add `~2` to the end of the branch.

`git diff dev solution~2 lib/STM32L432KC_RCC.c`

In this case, you'll likely see no difference since all the changes to the `lib/STM32L432KC_RCC.c` file happen on the final commit on the branch.