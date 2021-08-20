
if(FAST_MODULE_RealSense)
    message("-- Enabling real sense module.")
    if(WIN32)
        fast_download_dependency(realsense
                2.40.0
                f13f1435ef9498b0b53c869c2f389b66bcae220591625ab6059319a0c4acfa2e
                realsense2.lib
        )
    else()
        fast_download_dependency(realsense
                2.40.0
                19ab75148489d860ec2b73ad5edaad9000a6d155efb90ce6b4603aaf355bcd74
                librealsense2.so
        )
    endif()
endif()
