#===============================================================================
#Copyright (c) 2012-2013 QUALCOMM Austria Research Center GmbH.
#All Rights Reserved.
#
# This Vuforia(TM) sample application in source code form ("Sample Code") for
# the Vuforia Software Development Kit and/or Vuforia Extension for Unity
# (collectively, the "Vuforia SDK") may in all cases only be used in conjunction
# with use of the Vuforia SDK, and is subject in all respects to all of the
# terms and conditions of the Vuforia SDK License Agreement, which may be found
# at https://developer.vuforia.com/legal/license.
#
# By retaining or using the Sample Code in any manner, you confirm your
# agreement to all the terms and conditions of the Vuforia SDK License 
# Agreement. If you do not agree to all the terms and conditions of the Vuforia
# SDK License Agreement, then you may not retain or use any of the Sample Code
# in any manner.
#==============================================================================


# Build both ARMv5TE and ARMv7-A machine code.

APP_ABI := armeabi armeabi-v7a

# Set target Android API level to the application's minimum SDK version.

APP_PLATFORM := android-8

# This optional variable can be defined to either 'release' or
# 'debug'. This is used to alter the optimization level when
# building your application's modules.
#
# A 'release' mode is the default, and will generate highly
# optimized binaries. The 'debug' mode will generate un-optimized
# binaries which are much easier to debug.
#
# Note that it is possible to debug both 'release' and 'debug'
# binaries, but the 'release' builds tend to provide less information
# during debugging sessions: some variables are optimized out and
# can't be inspected, code re-ordering can make stepping through
# the code difficult, stack traces may not be reliable, etc...

# APP_OPTIM := release OR debug
