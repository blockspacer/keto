/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModuleManager.cpp
 * Author: ubuntu
 * 
 * Created on January 15, 2018, 3:12 PM
 */

#include <boost/filesystem/operations.hpp>
#include <condition_variable>

#include "keto/module/ModuleManager.hpp"
#include "keto/module/Constants.hpp"
#include "keto/module/Exception.hpp"
#include "include/keto/module/ModuleWrapper.hpp"
#include "include/keto/module/ModuleManagementInterface.hpp"


namespace keto {
namespace module {
    
namespace ketoEnv = keto::environment;

static std::shared_ptr<ModuleManager> singleton;

ModuleManager::ModuleManager() {
    this->currentState = inited;
    this->moduleDir = ketoEnv::EnvironmentManager::getInstance()->getEnv()->getInstallDir()
            / Constants::KETO_SHARE;
    if (!boost::filesystem::exists(this->moduleDir)) {
        BOOST_THROW_EXCEPTION(ModuleSharedDirException());
    }
    this->tmpDir = ketoEnv::EnvironmentManager::getInstance()->getEnv()->getInstallDir()
            / Constants::KETO_TMP;
    if (!boost::filesystem::exists(this->tmpDir)) {
        if (boost::filesystem::create_directories(this->tmpDir)) {
            BOOST_THROW_EXCEPTION(ModuleTmpDirException("Failed to create the directory : " +
                this->tmpDir.string()));
        }
    }
}

ModuleManager::~ModuleManager() {
}

static std::shared_ptr<ModuleManager>& ModuleManager::init() {
    if (!singleton) {
        singleton = std::make_shared<ModuleManager>();
    }
    return singleton;
}

static std::shared_ptr<ModuleManager>& ModuleManager::getInstance() {
    return singleton;
}

// module lifecycle methods
void ModuleManager::load() {
    // load 
    this->setState(State::loading);
    for(boost::filesystem::path& entry : boost::make_iterator_range(
            boost::filesystem::directory_iterator(this->moduleDir), {})) {
        load(entry,this->tmpDir);
    }
            
    this->setState(State::loaded);
    
}

void ModuleManager::monitor() {
    
    this->setState(State::monitoring);
    while(this->checkState() != State::terminated) {
        if (this->checkForReload()) {
            this->unload();
            this->load();
            this->setState(State::monitoring);
        }
    }
    
    
}

void ModuleManager::unload() {
    this->setState(State::unloading);
    std::map<boost::filesystem::path,std::shared_ptr<ModuleWrapper>> loadedLibraryModule;
    {
        std::lock_guard guard(this->classMutex);
        // copy the contents
        loadedLibraryModule = this->loadedLibraryModuleManagers;
        
        // clear the modules
        this->loadedModules.clear();
        this->loadedModuleManagementInterfaces.clear();
        this->loadedLibraryModuleManagers.clear();
        
    } 
    
    for (std::map<boost::filesystem::path,std::shared_ptr<ModuleWrapper>>::reverse_iterator it=loadedLibraryModule.rbegin(); 
            it!=loadedLibraryModule.rend(); ++it) {
        it->second->getModuleManagerInterface()->stop();
        it->second->unload();
    }
    
    // set the state
    this->setState(State::unloaded);
}


void ModuleManager::terminate() {
    std::unique_lock<std::mutex> uniqueLock(this->classMutex);
    this->currentState = State::terminated;
    this->stateCondition.notify_all();
}



ModuleManager::State ModuleManager::getState() {
    std::lock_guard<std::mutex> guard(this->classMutex);
    return this->currentState;
}

// modules methods
std::set<std::string> ModuleManager::listModules() {
    std::lock_guard guard(this->classMutex);
    std::vector<std::string> keys;
    std::transform(
        this->loadedModules.begin(),
        this->loadedModules.end(),
        std::back_inserter(keys),
        [](const std::map<auto,auto>::value_type &pair){return pair.first;});
    return keys;
}

std::shared_ptr<ModuleInterface> ModuleManager::getModule(
    const std::string& moduleName) {
    std::lock_guard guard(this->classMutex);
    return this->loadedModules[moduleName];
}


std::set<std::string> ModuleManager::listModuleManagementInterfaces() {
    std::lock_guard guard(this->classMutex);
    std::vector<std::string> keys;
    std::transform(
        this->loadedModuleManagementInterfaces.begin(),
        this->loadedModuleManagementInterfaces.end(),
        std::back_inserter(keys),
        [](const std::map<auto,auto>::value_type &pair){return pair.first;});
    return keys;
}

boost::shared_ptr<ModuleInterface> ModuleManager::getModuleManagementInterface(
    const std::string& moduleManagement) {
    std::lock_guard guard(this->classMutex);
    return this->loadedModuleManagementInterfaces[moduleManagement];
}


std::set<boost::filesystem::path> ModuleManager::listModuleFiles() {
    std::lock_guard guard(this->classMutex);
    std::set<boost::filesystem::path> keys;
    std::transform(
        this->loadedLibraryModuleManagers.begin(),
        this->loadedLibraryModuleManagers.end(),
        std::back_inserter(keys),
        [](const std::map<auto,auto>::value_type &pair){return pair.first;});
    return keys;
}


std::shared_ptr<ModuleWrapper> ModuleManager::getModuleWrapper(
    const boost::filesystem::path& modulePath) {
    std::lock_guard guard(this->classMutex);
    return this->loadedLibraryModuleManagers[modulePath];
}


void ModuleManager::load(const boost::filesystem::path& libraryPath) {
    // setup a module wrapper and load and start the module
    std::shared_ptr<ModuleWrapper> moduleWrapperPtr = std::make_shared<ModuleWrapper>(libraryPath,
            this->tmpDir);
    moduleWrapperPtr->load();
    std::shared_ptr<ModuleManagementInterface> moduleManagerInterfacePtr = 
            moduleWrapperPtr->getModuleManagerInterface();
    moduleManagerInterfacePtr->start();
    
    // add the module to the cointainers
    std::lock_guard guard(this->classMutex);
    this->loadedLibraryModuleManagers[libraryPath] = moduleWrapperPtr;
    this->loadedModuleManagementInterfaces[moduleWrapperPtr->getModuleManagerInterface()->getName()] = 
            moduleWrapperPtr->getModuleManagerInterface();
    std::vector<std::string> modules = moduleWrapperPtr->getModuleManagerInterface()->listModules();
    for (std::string name : modules) {
        this->loadedLibraryModuleManagers[name] = 
                moduleWrapperPtr->getModuleManagerInterface()->getModule(name);
    }
    
}


bool ModuleManager::checkForReload() {
    
    std::map<boost::filesystem::path,std::shared_ptr<ModuleWrapper>> loadedLibraryModuleManagers;
    {
        std::lock_guard guard(this->classMutex);
        // copy the contents
        loadedLibraryModuleManagers = this->loadedLibraryModuleManagers;
    }
    for (std::map<boost::filesystem::path,std::shared_ptr<ModuleWrapper>>::iterator it=loadedLibraryModuleManagers.begin(); 
            it!=loadedLibraryModuleManagers.end(); ++it) {
        if (it->second->requireReload()) {
            return true;
        }
    }
    return false;
}

ModuleManager::State ModuleManager::checkState() {
    std::unique_lock<std::mutex> uniqueLock(this->classMutex);
    this->stateCondition.wait_for(uniqueLock,std::chrono::microseconds(60));
    return this->currentState;
}

void ModuleManager::setState(const ModuleManager::State& state) {
    std::lock_guard guard(this->classMutex);
    if (this->currentState == State::terminated) {
        return;
    }
    this->currentState = state;
}

}
}