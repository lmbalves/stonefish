/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  USBL.h
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_USBL__
#define __Stonefish_USBL__

#include "comms/AcousticModem.h"
#include <random>

namespace sf
{
    //! An abstract class representing a USBL.
    class USBL : public AcousticModem
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param horizontalFOVDeg the horizontal beam angle [deg]
         \param verticalFOVDeg the vertical beam angle [deg]
         \param operatingRange the operating range [m]
         */
        USBL(std::string uniqueName, uint64_t deviceId, Scalar horizontalFOVDeg, Scalar verticalFOVDeg, Scalar operatingRange);
        
        //! A method performing internal comm state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to enable the auto pinging of connected transponder to monitor its position.
        /*!
         \param rate how often the ping should be sent (0 for continuous mode) [Hz]
         */
        void EnableAutoPing(Scalar rate = Scalar(0));
        
        //! A method to diable the autp pinging funtion.
        void DisableAutoPing();
        
        //! A method used to set the resolution of the measurements.
        /*!
         \param range the range measurement resolution [m]
         \param angleDeg the angle measurement resolution [deg]
         */
        void setResolution(Scalar range, Scalar angleDeg);

        //! A method used to set the noise characteristics of the device.
        /*!
         \param rangeDev standard deviation of the range measurement noise [m]
         \param horizontalAngleDevDeg standard deviation of the angle measurement noise [deg]
         \param verticalAngleDevDeg standard deviation of the angle measurement noise [deg]
         */
        void setNoise(Scalar rangeDev, Scalar horizontalAngleDevDeg, Scalar verticalAngleDevDeg);
        
        //! A method to get the current estimated position of transponders
        std::map<uint64_t, std::pair<Scalar, Vector3>>& getTransponderPositions(); 

        //! A method returning the type of the comm.
        CommType getType() const;
       
    protected:
        void ProcessMessages();
    
    private:
        bool ping;
        Scalar pingRate;
        Scalar pingTime;
        std::map<uint64_t, std::pair<Scalar, Vector3>> transponderPos;
        Scalar rangeRes;
        Scalar angleRes;
        bool noise;
        std::normal_distribution<Scalar> noiseRange;
        std::normal_distribution<Scalar> noiseHAngle;
        std::normal_distribution<Scalar> noiseVAngle;
        
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator;
    };
}
    
#endif
