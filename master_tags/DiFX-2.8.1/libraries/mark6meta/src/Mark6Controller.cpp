/*******************************************************************************
* Copyright (C) 2016  Max-Planck-Institut für Radioastronomie, Bonn, Germany 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id: Mark6DiskDevice.cpp 9271 2019-11-13 03:10:11Z HelgeRottmann $
// $HeadURL: $
// $LastChangedRevision: 9271 $
// $Author: HelgeRottmann $
// $LastChangedDate: 2019-11-13 04:10:11 +0100 (Mi, 13. Nov 2019) $
//
//============================================================================
#include "Mark6Controller.h"

#include <stdio.h>

using namespace std;

/**
 * Constructor
 */
Mark6Controller::Mark6Controller() {
    name_m = "";
    path_m = "";
    sysnum_m = "";
    order_m = -1;
}

/**
 * @return the device path of the controller
 */
std::string Mark6Controller::getPath() const {
    return path_m;
}

/** 
 * Set the device path of the controller
 * */
void Mark6Controller::setPath(std::string path) {
    this->path_m = path;
}
/**
 * @return the device name of the controller
 */
std::string Mark6Controller::getName() const {
    return name_m;
}
/** 
 * Set the device name of the controller
 * */
void Mark6Controller::setName(std::string name) {
    this->name_m = name;
}
/**
 * @return the instance number of the controller
 */
std::string Mark6Controller::getSysNum() const {
    return sysnum_m;
}

/** 
 * Set the device instance number of the controller
 * */
void Mark6Controller::setSysNum(std::string sysnum) {
    this->sysnum_m = sysnum;
}
/**
 * @return the device order of the controller
 */
int Mark6Controller::getOrder() const {
    return order_m;
}
/** 
 * Set the device order of the controller
 * */
void Mark6Controller::setOrder(int order) {
    this->order_m = order;
}


/**
 * Destructor
 */
Mark6Controller::~Mark6Controller() {
}

