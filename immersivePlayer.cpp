#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <ostream>
#include <string>
#include <type_traits>
#include "al/sound/al_SoundFile.hpp"

#include "miniShader/shaderUtility/shaderToSphere.hpp"
#include "adm-allo-player/mainplayer.hpp"


//IMMERSIVE SHADER PLAYER WITH DISTRIBUTED SHADERS FOR THE SPHERE + PLAYBACK FOR 54.1-CHANNEL ADM AUDIO




struct Common {};
class MyApp : public al::DistributedAppWithState<Common> {
public:
//USER CONFIGURATION HERE - EDIT PATHS / SETTINGS//
    //SET PATHS HERE //
  
  std::string fragName; 
  std::vector<std::string> fragOptions = {
      "CanyonScene.frag",
      "DunesScene.frag",
      "ScatScene.frag"
  };
  std::string shaderFolder = "../miniShader/demoShaders/"; // set to mistShaders or ripeShaders
  std::string sourceAudioFolderSelection;

  //USER CONTROLS HERE //
  float STARTING_TIME = 0.0f;
  float PLAYBACK_SPEED = 1.0f; // doesnt work for audio yet
   float audioGain = 0.0f;

// END USER CONFIGURATION //
//ADM PLAYER RELATED DONT TOUCH //
  adm_player adm_player_instance;
  MyApp() {
    adm_player_instance.toggleGUI(true); // disable GUI
    adm_player_instance.setSourceAudioFolder("../adm-allo-player/sourceAudio/");
  }
  // Graphics related DONT TOUCH //
  al::FileSelector selector;
  al::SearchPaths searchPaths;
  ShadedSphere shadedSphere;
  std::string vertPath;
  // std::string currentFragPath;
  std::vector<std::string> fragPathOptions;
  //
  // Audio related DONT TOUCH //
  //  al::SoundFilePlayer player;
  // Parameters DONT TOUCH //
  al::Parameter globalTime{"globalTime", "", STARTING_TIME, 0.0, 300.0};
  al::ParameterBool running{"running", "0", false};
  al::ParameterInt currentFragIndex{"currentFragIndex", "0", 0, 0, 10}; // for shader selection
  // al::ParameterString currentFragPathParam{"currentFragPathParam", "", ""}; // for shader selection via GUI
  // al::ParameterInt fragIndex{"fragIndex", "0", 0, 0, 10}; // for shader selection via GUI
  bool printTime = false;


  void onInit() override {
    adm_player_instance.onInit();


    parameterServer() << globalTime << running;
    // Graphics initialization
    searchPaths.addSearchPath(al::File::currentPath() + shaderFolder);

    al::FilePath vertPathSource = searchPaths.find("standard.vert");
    if (vertPathSource.valid()) {
      vertPath = vertPathSource.filepath();
      std::cout << "Found vertex shader: " << vertPath << std::endl;
    } else {
      std::cout << "Could not find vertex shader" << std::endl;
    }

    // Find fragment shaders listed in fragOptions and populate fragPathOptions.
    fragPathOptions.clear();
    // currentFragPath.clear();
    for (const auto &fragOption : fragOptions) {
      al::FilePath fragPathOptionSource = searchPaths.find(fragOption);
      if (fragPathOptionSource.valid()) {
        std::string fp = fragPathOptionSource.filepath();
        fragPathOptions.push_back(fp);
        std::cout << "Found fragment shader: " << fp << std::endl;
        // // Prefer the user-specified fragName if present, otherwise pick first found
        // if (currentFragPath.empty() || fragOption == fragName) {
        //   currentFragPath = fp;
        // }
      } else {
        std::cout << "Fragment shader not found in search paths: " << fragOption << std::endl;
      }
    }

    // if (currentFragPath.empty() && !fragPathOptions.empty()) {
    //   currentFragPath = fragPathOptions.front();
    // }

    // if (currentFragPath.empty()) {
    //   std::cerr << "No fragment shader found. Check shaderFolder and fragOptions." << std::endl;
    // }

    // Audio initialization
    // initializeAudio();
  }

  // void initializeAudio() {
  //
  // }

  void onCreate() override {
        adm_player_instance.onCreate();

    // Graphics setup
    shadedSphere.setSphere(15.0, 20);
    // currentFragPathParam = currentFragPath; // feed shader from primary to secondaries via paremeter
    if (!vertPath.empty() && !fragPathOptions[currentFragIndex].empty()) {
      shadedSphere.setShaders(vertPath, fragPathOptions[currentFragIndex]);
    } else {
      // Fallback: attempt to set whatever is available
      shadedSphere.setShaders(vertPath, fragPathOptions[currentFragIndex]);
    }
    shadedSphere.update();
  }
  void onAnimate(double dt) override {

    // Graphics animation
    if (running == true) {
      globalTime = globalTime + (dt * PLAYBACK_SPEED);
      if (printTime) {
        std::cout << globalTime << std::endl;
      }
    }
  }

  void onDraw(al::Graphics &g) override {
    if (isPrimary()) {
        adm_player_instance.onDraw(g);
    }

    // Graphics rendering
    g.clear(0.0);
    g.shader(shadedSphere.shader());
    shadedSphere.setUniformFloat("u_time", globalTime);
    shadedSphere.draw(g);
  }

  bool onKeyDown(const al::Keyboard &k) override {
    // Primary controls (graphics + shader selection)
    if (isPrimary()) {
      if (k.key() == ' ') {
        // Toggle both graphics and audio playback
        running = !running;
        // player.pause = !running;
        std::cout << (running ? "▶ Started" : "⏸ Paused") << " graphics" << std::endl;
        // return true;
      }

      // Select fragment shader via keys '1'..'9' (1 selects first found frag)
      if (k.key() >= '1' && k.key() <= '9') {
        globalTime = 0.0f; // reset time on shader switch
        int idx = static_cast<int>(k.key() - '1'); // '1'->0, '2'->1, ...
        if (idx < static_cast<int>(fragPathOptions.size())) {
          if (currentFragIndex == idx) {
            std::cout << "Fragment shader [" << (idx + 1) << "] already active: " << fragPathOptions[idx] << std::endl;
            return true; // no change needed
          } else {
          currentFragIndex = idx; // update parameter based on key
         
            if (!vertPath.empty() && !fragPathOptions[currentFragIndex].empty()) {
              shadedSphere.setShaders(vertPath, fragPathOptions[currentFragIndex]);
              shadedSphere.update();
            }
            std::cout << "Switched to fragment shader [" << (idx + 1) << "] " << fragPathOptions[currentFragIndex] << std::endl;
          }
        } else {
          std::cerr << "No fragment shader for key '" << static_cast<char>(k.key()) << "' (index " << (idx + 1) << ")" << std::endl;
        }
       
      }

          return adm_player_instance.onKeyDown(k);

    }

  }  

  void onSound(al::AudioIOData& io) override {
    adm_player_instance.onSound(io);

  }

private:
 


};

int main() {
  MyApp app;
 
  // Configure audio for 54 output channels, 0 input channels
  // Adjust sample rate and buffer size as needed. (samplerate, buffer size, output channels, input channels)
  app.configureAudio(48000, 512, 60, 0); // try 44.1k if there are sampling issuess
  
  
  app.start();
  return 0;
}
