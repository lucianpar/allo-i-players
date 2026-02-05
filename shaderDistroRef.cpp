#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sphere/al_SphereUtils.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "al_ext/statedistribution/al_CuttleboneDomain.hpp"
#include <iostream>
#include <string>

// spatial includes
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/ripple.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxUtility.hpp"
#include "al_ext/soundfile/al_SoundfileBuffered.hpp"
// #include "../meshMorph.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxMain.hpp"
// #include "../utility/loadAudioScene.hpp"
#include "../utility/parseObj.hpp"
// #include "../utility/staticSoundObject.hpp"
#include "../utility/soundObject.hpp"
#include "../utility/spatialPaths.hpp"
// #include "../imageToMesh.hpp"
// #include "../imageToSphere.hpp"
#include "../eoys-shader/shaderToSphere.hpp"
#include "../utility/attractors.hpp"
#include "../utility/creatures.hpp"
#include "../utility/imageColorToMesh.hpp"

#define nAgentsScene2 30

#define MAX_JELLIES 14

std::string slurp(const std::string &fileName);

al::Vec3f randomVec3f(float scale) {
  return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                   al::rnd::uniformS()) *
         scale;
}

struct Common {

  // double sceneTime;
  //  int sceneIndex;
  //  bool running;

  // scene 1
  al::Vec3f scene1Mesh[10000];
  al::Vec3f scene1BodyMesh[10000];

  // scene 2
  float blobPosX[nAgentsScene2];
  float blobPosY[nAgentsScene2];
  float blobPosZ[nAgentsScene2];
  float blobQuatW[nAgentsScene2];
  float blobQuatX[nAgentsScene2];
  float blobQuatY[nAgentsScene2];
  float blobQuatZ[nAgentsScene2];

  // scene 6
  float flicker;
  float jellyX[MAX_JELLIES];
  float jellyY[MAX_JELLIES];
  float jellyZ[MAX_JELLIES];
  float jellyQuatW[MAX_JELLIES];
  float jellyQuatX[MAX_JELLIES];
  float jellyQuatY[MAX_JELLIES];
  float jellyQuatZ[MAX_JELLIES];
};

class MyApp : public al::DistributedAppWithState<Common> {

  std::shared_ptr<al::CuttleboneDomain<Common>> cuttleboneDomain;

  al::FileSelector selector;
  al::SearchPaths searchPaths;

  ////spatial
  //
  al::Spatializer *spatializer{nullptr};
  al::Speakers speakerLayout;

  // al::ParameterBool pRunning{"running"};
  // al::Parameter pTime{"time", "", 0.0, 0.0, 10000}; 0ld stuff
  // al::ParameterInt pIndex{"index", "", 0, 0, 100};

  al::ParameterBool running{"running", "0", false};
  al::Parameter sceneTime{"sceneTime", "0", 0.0, 0.0, 10000};
  // al::Parameter sceneIndexParam{"sceneIndexParam", "0", 1, 0, 6};
  al::ParameterInt sceneIndex{"sceneIndex", "0", 0, 0, 10};

  //
public:
  al::Light light;
  al::Material material;
  // Global Time
  // int sceneIndex = 0;
  // int previousIndex = 0;
  double globalTime = 0;
  // double sceneTime;
  // bool running = false;
  float localTime;
  // float sceneIndex = 0;
  // double sceneTime = 64;
  // bool running = false;
  std::string objPath;
  std::string Song1Path;
  std::string Song2Path;
  std::string Song3Path;
  std::string Song4Path;
  std::string Song5Path;
  std::string Song6Path;
  std::string pointFragPath;
  std::string pointVertPath;
  std::string pointGeomPath;
  al::ShaderProgram pointShader;
  std::string vertPathScene3;
  std::string fragPathScene3;
  std::string vertPathScene4;
  std::string fragPathScene4;
  std::string vertPathScene5;
  std::string fragPathScene5;

  ShadedSphere shadedSphereScene3;
  ShadedSphere shadedSphereScene4;
  ShadedSphere shadedSphereScene5;

  // SCENE DECLERATIONS

  // SCENE 1 DECLARE START

  float rippleAmplitudeScene1 = 0.0;
  float rippleSpeedXScene1 = 0.2;
  float rippleSpeedYScene1 = 0.2;
  float rippleSpeedZScene1 = 0.3;
  float attractorSpeedScene1;
  float startingBodyAlpha = 0.00001;
  float bodyAlphaIncScene1 = 0.0;
  float pointSize = 5.0f; // Particle size
  float shellIncrementScene1;
  float particleIncrementScene1;

  // SCENE 1 sequencing

  float shellTurnsWhiteEvent = 15.0f;

  float particlesAppearEvent = 15.0f;

  float particlesSlowRippleEvent = 10.0f;

  float rippleSpeedUpEvent = 30.0f;

  float stopSpeedUpEvent = 62.0f;

  float startTurnShellBlack = 62.0f;

  float stopTurnShellBlack = 64.0f;

  float bodyCloudAppear = 65.0f;

  float bodyCloudMoveOut = 70.0f;

  // float startAttractor = 70.0f;

  float moveInEvent = 75.0;

  // scene 1 Meshes and Effects
  al::VAOMesh attractorMesh;
  al::VAOMesh bodyMesh;
  objParser newObjParser;
  Attractor mainAttractor;
  VertexEffectChain mainEffectChain;
  RippleEffect mainRippleY;
  RippleEffect mainRippleX;
  RippleEffect mainRippleZ;

  VertexEffectChain bodyEffectChain;
  ;
  ScatterEffect bodyScatter;

  // SCENE 1 DECLARE END

  // SCENE 2 DECLARE /////
  //  MESHES//
  al::VAOMesh blobMesh;
  al::VAOMesh starCreatureMesh;
  std::vector<al::Nav> blobs;
  std::vector<al::Vec3f> velocity;
  std::vector<al::Vec3f> force;
  Creature creature;
  // PARAMS

  // Creature creature;
  // std::vector<al::Vec3f> colorPallete = {
  //     {0.9f, 0.0f, 0.4}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};
  std::vector<al::Vec3f> colorPallete = {
      {0.9f, 0.0f, 0.4}, {0.8, 0.0, 0.46}, {0.969, 0.478, 0.102}};

  al::ParameterBool inSphereScene2{"inSphereScene2", "2", true};
  al::Parameter blobSeperationThresh{"blobSeperationThresh", "2", 2.0, 0.0,
                                     10.0};
  // al::Parameter scene2Boundary{"scene2Boundary", "2", 1.0, 0.0, 80.0};
  al::Parameter currentSpeedScene2{"currentSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter targetSpeedScene2{"targetSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter interpRateScene2{"interpRateScene2", "2", 0.01, 0.0, 1.0};
  // al::Parameter blobSizeScene2{"blobSizeScene2", "2", 1.8, 0.0, 10.0};
  float blobSizeScene2 = 1.5;
  float scene2Boundary = 1.0;
  ;

  // MESH EFFECTS//
  VertexEffectChain blobsEffectChain;
  RippleEffect blobsRippleX;
  RippleEffect blobsRippleY;
  RippleEffect blobsRippleZ;

  VertexEffectChain starEffectChain;
  RippleEffect starRipple;

  // WIND PIECE SEQUENCING EVENTS (SCENE 2)
  float windSpeedFastStart = 0.0f;
  float windSpeedSlow1 = 3.0f;
  float windSpeedSuperSlow = 3.5f;
  float windSpeedMedFast1 = 11.0f;
  float windSpeedSlow2 = 23.0f;
  float windScaleLarge1 = 31.0f;
  float windSpeedMed2 = 34.0f;
  float windSpeedFast2 = 39.0f;
  float windSpeedSlow3 = 44.0f;
  float windScaleSmall1 = 49.0f;
  float windSpeedMed3 = 49.0f;
  float windSpeedSlow4 = 54.0f;
  float windScaleHuge1 = 67.0f;
  float windSpeedMed4 = 77.0f;
  float windSpeedSlower5 = 93.0f;
  float windScaleDownStart = 96.0f;
  float windSpeedFast3 = 99.0f;
  float windSpeedSlow5 = 101.0f;
  float windSpeedFast4 = 104.0f;
  float windSpeedSlow6 = 107.0f;
  float windSpeedMed5 = 117.0f;
  float windSizeFastTiny = 123.0f;
  float windSpeedSlow7 = 126.0f;
  float windScaleHuge2 = 133.0f;
  float windSpeedMed6 = 142.0f;
  float windScaleReturn = 146.0f;
  float windSpeedSlow8 = 150.0f;
  float windSpeedFast5 = 164.0f;
  float windSpeedSuperSlow2 = 175.0f;
  float windScaleFadeStart = 181.0f;
  float windSpeedMedSlow = 183.0f;
  float windFinalSlowdown = 199.0f;

  //// SCENE 2 END DECLARATIONS ////

  // SCENE 6 DECLARE START

  al::VAOMesh jellyCreatureMesh;
  std::vector<al::Nav> jellies;

  // === Scene 6 PARAMETERS ===
  al::Parameter scene6Boundary{"scene6Boundary", "", 50.0f, 0.0f, 100.0f};
  al::ParameterBool inSphereScene6{"inSphereScene6", "", true};
  al::Parameter jellieseperationThresh{"jellieseperationThresh", "", 4.0f, 0.0f,
                                       20.0f};
  al::Parameter jelliesSpeedScene6{"jelliesSpeedScene6", "", 3.0f, 0.0f, 10.0f};
  al::Parameter jelliesizeScene2{"jelliesizeScene2", "", 5.0f, 0.0f, 20.0f};
  al::Parameter pointSizeScene6{"pointSizeScene6", "", 2.5f, 0.1f, 10.0f};
  al::Parameter scene6pulseSpeed{"scene6pulseSpeed", "", 0.4f, 0.0f, 5.0f};
  al::Parameter scene6pulseAmount{"scene6pulseAmount", "", 0.2f, 0.0f, 5.0f};

  // Scene 6 Effects
  AutoPulseEffect jellyPulse;
  RippleEffect jellyRippleY;
  RippleEffect jellyRippleX;
  VertexEffectChain jellyEffectChain;

  /// SCENE 6 DECLARE END

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    parameterServer() << running << sceneTime << sceneIndex;

    // SPATIAL STUFF
    audioIO().channelsBus(1);
    if (al::sphere::isSphereMachine())
      speakerLayout = al::AlloSphereSpeakerLayoutCompensated();
    else
      speakerLayout = al::StereoSpeakerLayout();

    spatializer = new al::AmbisonicsSpatializer(speakerLayout); //, 3, 1, 1);
    spatializer->compile();

    // FILE PATH STUFF
    searchPaths.addAppPaths();
    searchPaths.addRelativePath("../..");
    // searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");

    // POINT SHADER PATHS

    // probaboy can be in if primary

    al::FilePath frag = searchPaths.find("point-fragment.glsl");
    if (frag.valid()) {
      pointFragPath = frag.filepath();
      std::cout << "Found file at: " << pointFragPath << std::endl;
    } else {
      std::cout << "couldnt find point frag in path" << std::endl;
    }
    al::FilePath geom = searchPaths.find("point-geometry.glsl");
    if (frag.valid()) {
      pointGeomPath = geom.filepath();
      std::cout << "Found file at: " << pointGeomPath << std::endl;
    } else {
      std::cout << "couldnt find point geom in path" << std::endl;
    }
    al::FilePath vert = searchPaths.find("point-vertex.glsl");
    if (vert.valid()) {
      pointVertPath = vert.filepath();
      std::cout << "Found file at: " << pointVertPath << std::endl;
    } else {
      std::cout << "couldnt find point vert in path" << std::endl;
    }

    al::FilePath objFP = searchPaths.find("floatBody.obj");
    if (objFP.valid()) {
      objPath = objFP.filepath();
      std::cout << "Found file at: " << objPath << std::endl;
    } else {
      std::cout << "couldnt find basemesh in path" << std::endl;
    }

    // AAUDIO PATHS

    if (isPrimary()) {
      al::FilePath audio1Fp = searchPaths.find("Song1.wav");
      if (audio1Fp.valid()) {
        Song1Path = audio1Fp.filepath();
        std::cout << "Found file at: " << Song1Path << std::endl;
      } else {
        std::cout << "couldnt find song1 in path" << std::endl;
      }
      al::FilePath audio2Fp = searchPaths.find("Song2.wav");
      if (audio2Fp.valid()) {
        Song2Path = audio2Fp.filepath();
        std::cout << "Found file at: " << Song2Path << std::endl;
      } else {
        std::cout << "couldnt find song2 in path" << std::endl;
      }
      al::FilePath audio3Fp = searchPaths.find("Song3.wav");
      if (audio3Fp.valid()) {
        Song3Path = audio3Fp.filepath();
        std::cout << "Found file at: " << Song3Path << std::endl;
      } else {
        std::cout << "couldnt find song3 in path" << std::endl;
      }
      al::FilePath audio4Fp = searchPaths.find("Song4.wav");
      if (audio4Fp.valid()) {
        Song4Path = audio4Fp.filepath();
        std::cout << "Found file at: " << Song4Path << std::endl;
      } else {
        std::cout << "couldnt find song4 in path" << std::endl;
      }
      al::FilePath audio5Fp = searchPaths.find("Song5.wav");
      if (audio5Fp.valid()) {
        Song5Path = audio5Fp.filepath();
        std::cout << "Found file at: " << Song5Path << std::endl;
      } else {
        std::cout << "couldnt find song5 in path" << std::endl;
      }
      al::FilePath audio6Fp = searchPaths.find("Song6.wav");
      if (audio6Fp.valid()) {
        Song6Path = audio6Fp.filepath();
        std::cout << "Found file at: " << Song6Path << std::endl;
      } else {
        std::cout << "couldnt find song6 in path" << std::endl;
      }
    }

    // scene 3 paths
    al::FilePath vertPath3 = searchPaths.find("standard.vert");
    if (vertPath3.valid()) {
      vertPathScene3 = vertPath3.filepath();
      std::cout << "Found file at: " << vertPathScene3 << std::endl;
    } else {
      std::cout << "couldnt find ver scene 4 in path" << std::endl;
    }
    al::FilePath fragPath3 =
        searchPaths.find("SpruceMain.frag"); // switch to spruce main
    if (fragPath3.valid()) {
      fragPathScene3 = fragPath3.filepath();
      std::cout << "Found file at: " << fragPathScene3 << std::endl;
    } else {
      std::cout << "couldnt find frag scene 4 in path" << std::endl;
    }

    // scene 4 paths :
    al::FilePath vertPath4 = searchPaths.find("standard.vert");
    if (vertPath4.valid()) {
      vertPathScene4 = vertPath4.filepath();
      std::cout << "Found file at: " << vertPathScene4 << std::endl;
    } else {
      std::cout << "couldnt find ver scene 4 in path" << std::endl;
    }
    al::FilePath fragPath4 = searchPaths.find("FoamMain.frag");
    if (fragPath4.valid()) {
      fragPathScene4 = fragPath4.filepath();
      std::cout << "Found file at: " << fragPathScene4 << std::endl;
    } else {
      std::cout << "couldnt find frag scene 4 in path" << std::endl;
    }

    // scene 5 paths

    al::FilePath vertPath5 = searchPaths.find("standard.vert");
    if (vertPath5.valid()) {
      vertPathScene5 = vertPath5.filepath();
      std::cout << "Found file at: " << vertPathScene5 << std::endl;
    } else {
      std::cout << "couldnt find vert scene 5 in path" << std::endl;
    }
    al::FilePath fragPath5 = searchPaths.find("CrushedMain.frag");
    if (fragPath5.valid()) {
      fragPathScene5 = fragPath5.filepath();
      std::cout << "Found file at: " << fragPathScene5 << std::endl;
    } else {
      std::cout << "couldnt find frag scene 5 in path" << std::endl;
    }
  }

  void onCreate() override {

    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));

    cuttleboneDomain = al::CuttleboneDomain<Common>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERRor: Cuttlebone not started" << std::endl;
    }

    nav().pos(al::Vec3d(0, 0, 0));
    // sequencer().playSequence();

    // if (sceneIndex == 1) {
    //  std::string path =
    //  localAssetPath("../softlight-sphere-new/audio/Song2.wav");

    // SCENE 1
    createScene1();

    // SCENE 2
    createScene2();
    // scene 3
    shadedSphereScene3.setSphere(15.0, 20);
    shadedSphereScene3.setShaders(vertPathScene3, fragPathScene3);
    shadedSphereScene3.update();

    // scene 4
    shadedSphereScene4.setSphere(15.0, 20);
    shadedSphereScene4.setShaders(vertPathScene4, fragPathScene4);
    shadedSphereScene4.update();
    // scene 5
    shadedSphereScene5.setSphere(15.0, 20);
    shadedSphereScene5.setShaders(vertPathScene5, fragPathScene5);
    shadedSphereScene5.update();

    // scene 6
    createScene6();

    double g = 0.7;
    float a = 1.4;
    float b = 1.6;
    float c = 1.0;
    float d = 0.7;

    if (isPrimary()) {
      sequencer1().add<SoundObject>(0, 44000).set(

          1, 1, 1, 0.1, true, Song1Path.c_str(),
          [&](double t, const al::Vec3f &p) -> al::Vec3f {
            return al::Vec3f(
                // body of lambda logic. will replace this will header calls
                (sin(a * p.y) + c * cos(a * p.x)),
                (sin(b * p.x) + d * cos(b * p.y)), p.z);
          });

      // sequencer2().add<SoundObject>(0, 44000).set(

      //     Song2Path.c_str());
      // sequencer3().add<SoundObject>(0, 44000).set(

      //     Song3Path.c_str());
      // sequencer4().add<SoundObject>(0, 44000).set(

      //     Song4Path.c_str());
      // sequencer5().add<SoundObject>(0, 44000).set(

      //     Song5Path.c_str());
      // sequencer6().add<SoundObject>(0, 44000).set(

      //     Song6Path.c_str());
    }
  }

  bool onKeyDown(const al::Keyboard &k) override {

    if (isPrimary()) {

      if (k.key() == ' ' && running == false) {
        running = true;
        std::cout << "started running" << std::endl;
      } else if (k.key() == ' ' && running == true) {
        running = false;
        std::cout << "stopped running" << std::endl;
      } else {
        //   sceneTime = 0;
        //   std::cout << "reset scene time to 0" << std::endl;
      }
      if (k.key() == '1') {
        sceneIndex = 1;
        globalTime = 0.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 1 << "global time: " << globalTime
                  << std::endl;

        sequencer1().playSequence();

        return true;
      }
      if (k.key() == '2') {
        sceneIndex = 2;
        globalTime = 119.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 2 << "global time: " << globalTime
                  << std::endl;

        sequencer2().playSequence();

        return true;
      }
      if (k.key() == '3') {
        sceneIndex = 3;
        globalTime = 335.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 3 << "global time: " << globalTime
                  << std::endl;

        sequencer3().playSequence();
        return true;
      }
      if (k.key() == '4') {
        sceneIndex = 4;
        globalTime = 444.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 4 << "global time: " << globalTime
                  << std::endl;

        sequencer4().playSequence();
        return true;
      }
      if (k.key() == '5') {
        sceneIndex = 5;
        globalTime = 936.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 5 << "global time: " << globalTime
                  << std::endl;

        sequencer5().playSequence();
        return true;
      }
      if (k.key() == '6') {
        sceneIndex = 6;
        globalTime = 1105.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 6 << "global time: " << globalTime
                  << std::endl;

        sequencer6().playSequence();
        return true;
      }
      // sceneIndexParam.set(sceneIndex);
      return false;
    }
    // sceneIndex = sceneIndexParam;
  }

  void onAnimate(double dt) override {

    // boiler plate for every scene / main template
    // if (!isPrimary()) {
    //   running = state().running;
    //   running = pRunning.get();
    // } else {
    //   state().running = running;
    //   pRunning.set(running);
    // }

    // std::cout << "running : " << state().running << std::endl;
    // std::cout << "index : " << state().sceneIndex << std::endl;
    // std::cout << "time : " << state().sceneTime << std::endl;

    if (running == true) {

      if (isPrimary()) {
        globalTime += dt;
        // // time : " << globalTime << std::endl;
        sceneTime = sceneTime + dt;
        if (globalTime >= 0.0 && globalTime < 0.0 + dt) {
          sceneIndex = 1;
          sceneTime = 0.0;
          sequencer1().playSequence();
          std::cout << "started scene 1" << std::endl;
        } else if (globalTime >= 119.0 && globalTime < 119.0 + dt) {
          sceneIndex = 2;
          sceneTime = 0.0;
          sequencer2().playSequence();
          std::cout << "started scene 2" << std::endl;
        } else if (globalTime >= 335.0 && globalTime < 335.0 + dt) {
          sceneIndex = 3;
          sceneTime = 0.0;
          sequencer3().playSequence();
          std::cout << "started scene 3" << std::endl;
        } else if (globalTime >= 444.0 && globalTime < 444.0 + dt) {
          sceneIndex = 4;
          sceneTime = 0.0;
          sequencer4().playSequence();
          std::cout << "started scene 4" << std::endl;
        } else if (globalTime >= 936.0 && globalTime < 936.0 + dt) {
          sceneIndex = 5;
          sceneTime = 0.0;
          sequencer5().playSequence();
          std::cout << "started scene 5" << std::endl;
        } else if (globalTime >= 1105.0 && globalTime < 1105.0 + dt) {
          sceneIndex = 6;
          sceneTime = 0.0;
          sequencer6().playSequence();
          std::cout << "started scene 6" << std::endl;
        }
      } else {
        // sceneTime = localTime;
      }
      shadedSphereScene3.update();
      shadedSphereScene4.update();
      shadedSphereScene5.update();
      // move these into conditions ^

      // scene 1
      if (sceneIndex == 1) {
        animateScene1(dt);
      }
      if (sceneIndex == 2) {
        animateScene2(dt);
      }

      if (sceneIndex == 6) {
        animateScene6(dt);
      }
    }
  }

  void onDraw(al::Graphics &g) override {
    // end of boilerplate

    if (running == true) {

      // SCENE 1 DRAW /////
      if (sceneIndex == 1) {
        drawScene1(g);
      }
      if (sceneIndex == 2) {
        drawScene2(g);
      }
      if (sceneIndex == 3) {
        // if (globalTime > 0) {
        g.clear(0.0);

        g.shader(shadedSphereScene3.shader());
        shadedSphereScene3.setUniformFloat("u_time", sceneTime);

        shadedSphereScene3.draw(g);
        shadedSphereScene3.update();
      }
      if (sceneIndex == 4) {
        g.clear(0.0);

        g.shader(shadedSphereScene4.shader());
        shadedSphereScene4.setUniformFloat("u_time", sceneTime);

        shadedSphereScene4.draw(g);
      }

      if (sceneIndex == 5) {
        g.clear(0.0);

        g.shader(shadedSphereScene5.shader());
        shadedSphereScene5.setUniformFloat("u_time", sceneTime);

        shadedSphereScene5.draw(g);
      }

      if (sceneIndex == 6) {
        drawScene6(g);
      }
    }
  }

  void createScene1() {
    newObjParser.parse(objPath, bodyMesh);
    bodyMesh.translate(0, 3.5, -4);
    for (int i = 0; i < bodyMesh.vertices().size(); ++i) {
      bodyMesh.color(
          1.0, 0.6, 0.3,
          startingBodyAlpha); // Orange particles with alpha transparency
      bodyMesh.texCoord(1.0f, 0.0f);
    }
    bodyMesh.update();
    bodyMesh.primitive(al::Mesh::POINTS);
    bodyMesh.update();

    // Initialize attractor
    al::addSphere(attractorMesh, 10.0f, 100, 100);
    attractorMesh.primitive(al::Mesh::LINES); // switch back to lines
    for (int i = 0; i < attractorMesh.vertices().size(); ++i) {
      attractorMesh.color(1.0, 0.6, 0.2, 0.9);
    }
    std::cout << "made attractor sphere with vertices # : "
              << attractorMesh.vertices().size() << std::endl;

    // SET EFFEFCTS
    bodyScatter.setBaseMesh(bodyMesh.vertices());
    bodyScatter.setParams(0.5, 20.0);
    bodyScatter.setScatterVector(bodyMesh);
    mainRippleX.setParams(4, 0.2, 4.0, 'y');
    mainRippleX.setParams(4, 0.2, 6.0, 'x');
    mainRippleX.setParams(4, 0.2, 5.0, 'z');

    mainEffectChain.pushBack(&mainRippleX);
    mainEffectChain.pushBack(&mainRippleY);
    mainEffectChain.pushBack(&mainRippleZ);

    bodyEffectChain.pushBack(&bodyScatter);

    bodyScatter.triggerOut(true, bodyMesh);

    bodyMesh.update();

    attractorMesh.update();
  }

  void animateScene1(double dt) {

    // animate vertices
    if (isPrimary()) {
      if (sceneTime < particlesSlowRippleEvent) {
        mainAttractor.processThomas(attractorMesh, sceneTime, 0);
      }

      if (sceneTime >= particlesSlowRippleEvent &&
          sceneTime <= rippleSpeedUpEvent) {
        attractorSpeedScene1 = 0.00005;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 20 * 0.00001, -5 * 0.00002);
      }

      if (sceneTime >= rippleSpeedUpEvent && sceneTime <= stopSpeedUpEvent) {
        attractorSpeedScene1 = 0.00015;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 40 * 0.0001, -5 * 0.00003);
      }
      if (sceneTime >= stopSpeedUpEvent && sceneTime <= moveInEvent) {
        attractorSpeedScene1 = 0.00005;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 300 * 0.00005, -15 * 0.00005);
      }

      if (sceneTime >= stopSpeedUpEvent) {
        attractorSpeedScene1 = 0.00001;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        // attractorMesh.translate(

        //     0, 0, -10 * 0.0002);
        // mainEffectChain.process(attractorMesh, sceneTime); //ripple
        // commented out
      }

      // body mesh effects
      if (sceneTime >= bodyCloudAppear) {
        if (bodyAlphaIncScene1 < 1.0) {
          bodyAlphaIncScene1 += 0.0000000000000001;
        }
      }
      if (sceneTime >= moveInEvent) {
        attractorSpeedScene1 = 0.00001;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        bodyScatter.setParams(6, 20.0);
        bodyScatter.triggerIn(true);

        attractorMesh.translate(

            0, 4500.5 * 0.00001, -4000.0 * 0.00001);
        // attractorMesh.scale(0.996);
      }
      bodyEffectChain.process(bodyMesh, sceneTime);
    }

    if (isPrimary()) {
      for (int i = 0; i < attractorMesh.vertices().size(); i++)
        state().scene1Mesh[i] = attractorMesh.vertices()[i];
      for (int i = 0; i < bodyMesh.vertices().size(); i++)
        state().scene1BodyMesh[i] = bodyMesh.vertices()[i];
    } else {
      for (int i = 0; i < attractorMesh.vertices().size(); i++)
        attractorMesh.vertices()[i] = state().scene1Mesh[i];
      for (int i = 0; i < bodyMesh.vertices().size(); i++)
        bodyMesh.vertices()[i] = state().scene1BodyMesh[i];
    }

    attractorMesh.update();
    bodyMesh.update();

    // SCENE 1 ANIMATE END
  }
  void drawScene1(al::Graphics &g) {
    glEnable(GL_PROGRAM_POINT_SIZE);

    if (sceneTime < shellTurnsWhiteEvent) {
      shellIncrementScene1 = ((sceneTime) / (shellTurnsWhiteEvent));
      g.clear(1.0 - shellIncrementScene1);
    }
    if (sceneTime >= shellTurnsWhiteEvent) {
      g.clear(0.0);
    }
    g.depthTesting(true);
    g.blending(true);
    g.blendAdd();

    g.pointSize(pointSize);
    g.meshColor();
    g.draw(attractorMesh);

    if (sceneTime >= bodyCloudAppear) {
      g.shader(pointShader);
      pointShader.uniform("pointSize", 0.01);
      pointShader.uniform("inputColor",
                          al::Vec4f(1.0, 0.6, 0.3, bodyAlphaIncScene1));
      g.draw(bodyMesh);
      g.shader();
    }
    // } else {
    //   g.clear(0.0);
  }

  void createScene2() {
    parameterServer() << blobSeperationThresh;
    // parameterServer() << scene2Boundary;
    parameterServer() << currentSpeedScene2;
    parameterServer() << targetSpeedScene2;
    parameterServer() << interpRateScene2;
    parameterServer() << inSphereScene2;
    // parameterServer() << blobSizeScene2;
    //  if (isPrimary()) {
    addSphere(blobMesh, 1.8, 40, 40);
    blobMesh.primitive(
        al::Mesh::LINE_STRIP_ADJACENCY); // test if i like lines of triangles
                                         // more in the sphere
    blobMesh.generateNormals();
    creature.addStarfish(starCreatureMesh);
    starCreatureMesh.update();

    for (int b = 0; b < nAgentsScene2; ++b) {
      al::Nav p;
      blobs.push_back(p);
    }

    // Only assign random positions and orientations on primary node
    if (isPrimary()) {
      for (int b = 0; b < nAgentsScene2; ++b) {
        blobs[b].pos() = randomVec3f(5.0f);
        blobs[b]
            .quat()
            .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
                 al::rnd::uniformS())
            .normalize();
      }
    }

    blobMesh.update();

    blobsRippleX.setParams(0.2, 0.1, 1.0, 'x');
    blobsRippleZ.setParams(0.4, 0.1, 1.0, 'z');
    blobsEffectChain.pushBack(&blobsRippleZ);
    blobsEffectChain.pushBack(&blobsRippleX);
    starRipple.setParams(1.0, 1.0, 1.0, 'z');
    starEffectChain.pushBack(&starRipple);
    blobMesh.update();
  }

  void animateScene2(double dt) {
    if (isPrimary()) {

      if (sceneTime < windSpeedSlow1) {
        targetSpeedScene2 = 3.0f;
        scene2Boundary = 12.0f; // Start open and fast
      } else if (sceneTime < windSpeedSuperSlow) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedMedFast1) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 3.0f; // Tight = appear large
      } else if (sceneTime < windSpeedSlow2) {
        targetSpeedScene2 = 5.0f; // Emphasize size release
        scene2Boundary = 18.0f;   // Open quickly = appear smaller
      } else if (sceneTime < windScaleLarge1) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 4.0f;
      } else if (sceneTime < windSpeedMed2) {
        targetSpeedScene2 = 5.5f; // Burst movement with scale jump
        scene2Boundary = 20.0f;
      } else if (sceneTime < windSpeedFast2) {
        targetSpeedScene2 = 4.0f;
        scene2Boundary = 25.0f;
      } else if (sceneTime < windSpeedMed3) {
        targetSpeedScene2 = 1.2f;
        scene2Boundary = 5.0f; // Dramatic zoom-in
      } else if (sceneTime < windSpeedSlow4) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 8.0f;
      } else if (sceneTime < windSpeedMed4) {
        targetSpeedScene2 = 4.5f;
        scene2Boundary = 22.0f; // Blowout moment
      } else if (sceneTime < windSpeedSlower5) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedFast3) {
        targetSpeedScene2 = 3.5f;
        scene2Boundary = 15.0f;
      } else if (sceneTime < windSpeedSlow5) {
        targetSpeedScene2 = 0.4f;
        scene2Boundary = 3.5f;
      } else if (sceneTime < windSpeedFast4) {
        targetSpeedScene2 = 3.0f;
        scene2Boundary = 18.0f;
      } else if (sceneTime < windSpeedMed5) {
        targetSpeedScene2 = 0.6f;
        scene2Boundary = 10.0f;
      } else if (sceneTime < windSpeedSlow7) {
        targetSpeedScene2 = 6.0f; // Blast outward
        scene2Boundary = 30.0f;
      } else if (sceneTime < windSpeedMed6) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 5.0f;
      } else if (sceneTime < windScaleReturn) {
        targetSpeedScene2 = 2.2f;
        scene2Boundary = 14.0f;
      } else if (sceneTime < windSpeedSlow8) {
        targetSpeedScene2 = 0.3f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedFast5) {
        targetSpeedScene2 = 4.0f;
        scene2Boundary = 22.0f;
      } else if (sceneTime < windSpeedSuperSlow2) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 5.0f;
      } else if (sceneTime < windScaleFadeStart) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 3.0f;
      } else if (sceneTime < windSpeedMedSlow) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 2.0f; // Ultra-tight presence
      } else if (sceneTime < windFinalSlowdown) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 10.0f;
      } else {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 800.0f; // Full release drift
        // blobsEffectChain.clear(); // Optional: end FX
      }
    }
    // Animate all blobs
    if (isPrimary()) {
      for (int i = 0; i < blobs.size(); ++i) {
        al::Vec3f pos = blobs[i].pos();

        if (pos.mag() >= scene2Boundary) {
          blobs[i].faceToward(al::Vec3f(0),
                              0.01); // slow turn rate to avoid jitter
          blobs[i].moveF(blobSizeScene2);
        }
        // for setting state for renderers
        blobs[i].moveF(targetSpeedScene2 * 15.0f); // use smoothed speed
        blobs[i].step(dt);
        state().blobPosX[i] = blobs[i].pos().x;
        state().blobPosY[i] = blobs[i].pos().y;
        state().blobPosZ[i] = blobs[i].pos().z;
        state().blobQuatW[i] = blobs[i].quat().w;
        state().blobQuatX[i] = blobs[i].quat().x;
        state().blobQuatY[i] = blobs[i].quat().y;
        state().blobQuatZ[i] = blobs[i].quat().z;
      }
      blobsEffectChain.process(blobMesh, sceneTime);
      blobMesh.generateNormals();
      blobMesh.update();

      starEffectChain.process(starCreatureMesh, sceneTime);
      starCreatureMesh.generateNormals();
      starCreatureMesh.update();

      // THIS PROCESSING MIGHT NEED TO UPDATE OUTSIDE PRIMARY AS WELL?
    }
    if (!isPrimary()) {
      // updating pos and turning state
      for (int i = 0; i < blobs.size(); ++i) {
        blobs[i].pos().set(state().blobPosX[i], state().blobPosY[i],
                           state().blobPosZ[i]);

        // SETTING QUAT

        blobs[i].quat().set(state().blobQuatW[i], state().blobQuatX[i],
                            state().blobQuatY[i], state().blobQuatZ[i]);

        // blobs[i]
        //     .quat()
        //     .set(blobs[i].pos().x, blobs[i].pos().y,
        //     blobs[i].pos().z, 1.0f) .normalize(); // more efficient but
        //     less interesting
        // turning weird? come here
        blobMesh.update();
        starCreatureMesh.update();
      }
    }
  }

  void drawScene2(al::Graphics &g) {
    std::cout << "scene 2 draw " << std::endl;
    g.clear(0.0, 0.0, 0.09 + ((sceneTime / (334.0 - 118.0)) * 0.15));
    g.light(light);

    g.blendTrans();
    g.depthTesting(true);

    g.lighting(true); // flashing / flickering -- comment this out if needed
    // lighting from karl's example
    light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
    light.ambient(al::RGB(0.5, (1.0), 1.0));
    light.diffuse(al::RGB(1, 1, 0.5));

    material.specular(light.diffuse());
    material.shininess(50);
    g.material(material);

    for (int i = 0; i < blobs.size(); ++i) {
      al::Vec3f newColor = colorPallete[i % 3];

      g.pushMatrix();
      g.translate(blobs[i].pos());
      g.rotate(blobs[i].quat());
      g.scale(1.5);
      if (i % 2 == 1) {
        g.color(newColor.x, newColor.y, newColor.z,
                0.3 + (sin(sceneTime * 2.0) * 0.1));
        g.draw(blobMesh);
      } else {
        g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4,
                0.4 + (sin(sceneTime * 0.6) * 0.1));
        g.draw(starCreatureMesh);
      }
      g.popMatrix();
    }
  }

  void createScene6() {
    creature.makeJellyfish(jellyCreatureMesh, 0.6f, 72, 48, 8, 40, 40, 4.0f,
                           0.25f, 5.5f, 1.0f);
    for (int i = 0; i < jellyCreatureMesh.vertices().size(); ++i) {
      jellyCreatureMesh.color(1.0, 1, 1, 1);
      jellyCreatureMesh.texCoord(1.0f, 0.0f);
    }
    jellyCreatureMesh.scale(jelliesizeScene2);
    jellyCreatureMesh.primitive(al::Mesh::POINTS);
    jellyCreatureMesh.generateNormals();
    jellyPulse.setBaseMesh(jellyCreatureMesh.vertices());
    jellyPulse.setParams(scene6pulseSpeed, scene6pulseAmount, 1);
    jellyEffectChain.pushBack(&jellyPulse);
    jellyCreatureMesh.update();

    for (int b = 0; b < MAX_JELLIES; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      jellies.push_back(p);
      state().jellyX[b] = p.pos().x;
      state().jellyY[b] = p.pos().y;
      state().jellyZ[b] = p.pos().z;
      state().jellyQuatW[b] = p.quat().w;
      state().jellyQuatX[b] = p.quat().x;
      state().jellyQuatY[b] = p.quat().y;
      state().jellyQuatZ[b] = p.quat().z;
    }
  }

  void animateScene6(double dt) {
    if (isPrimary()) {

      if (sceneTime < 10.0f) {
        jelliesSpeedScene6 = 0.3f;
        scene6Boundary = 15.0f;

      } else if (sceneTime < 60.0f) {
        jelliesSpeedScene6 = 3.0f;
        scene6Boundary = 15.0f;

      } else if (sceneTime < 64.0f) {
        jelliesSpeedScene6 = 5.5f;
        scene6Boundary = 16.0f;

      } else if (sceneTime < 86.0f) {
        jelliesSpeedScene6 = 1.0f;
        scene6Boundary = 14.0f;

      } else if (sceneTime < 94.0f) {
        jelliesSpeedScene6 = 6.5f;
        scene6Boundary = 25.0f;

      } else if (sceneTime < 108.0f) {
        jelliesSpeedScene6 = 0.8f;
        scene6Boundary = 10.0f;

      } else if (sceneTime < 135.0f) {
        jelliesSpeedScene6 = 0.3f;
        scene6Boundary = 5.0f;

      } else if (sceneTime < 155.0f) {
        jelliesSpeedScene6 = 4.0f;
        scene6Boundary = 18.0f;

      } else if (sceneTime < 265.0f) {
        jelliesSpeedScene6 = 0.2f;
        scene6Boundary = 2.0f;

      } else if (sceneTime < 300.0f) {
        jelliesSpeedScene6 = 3.2f;
        scene6Boundary = 20.0f;

      } else if (sceneTime < 372.0f) {
        jelliesSpeedScene6 = 6.0f;
        scene6Boundary = 14.0f;

      } else if (sceneTime < 405.0f) {
        jelliesSpeedScene6 = 4.0f;
        scene6Boundary = 10.0f;

      } else if (sceneTime < 440.0f) {
        jelliesSpeedScene6 = 2.2f;
        scene6Boundary = 6.0f;

      } else if (sceneTime < 470.0f) {
        jelliesSpeedScene6 = 3.5f;
        scene6Boundary = 14.0f;

      } else if (sceneTime < 495.0f) {
        jelliesSpeedScene6 = 1.0f;
        scene6Boundary = 8.0f;

      } else {

        jelliesSpeedScene6 = 0.2f;
        scene6Boundary = 60; // slow dissolve out
      }

      for (int i = 0; i < jellies.size(); ++i) {
        float t = globalTime + i * 10.0f;
        float wobbleAmount = 0.01f * std::sin(t * 0.7f);
        jellies[i].turnF(0.004f + wobbleAmount);
        if (jellies[i].pos().mag() > scene6Boundary.get())
          jellies[i].faceToward(al::Vec3f(0), 0.005f);
        jellies[i].moveF(jelliesSpeedScene6.get() * 2.0);
        jellies[i].step(dt);

        state().jellyX[i] = jellies[i].pos().x;
        state().jellyY[i] = jellies[i].pos().y;
        state().jellyZ[i] = jellies[i].pos().z;
        state().jellyQuatW[i] = jellies[i].quat().w;
        state().jellyQuatX[i] = jellies[i].quat().x;
        state().jellyQuatY[i] = jellies[i].quat().y;
        state().jellyQuatZ[i] = jellies[i].quat().z;

        state().flicker =
            0.25f +
            0.05f * std::sin(sceneTime * 2.0); // move back outside is primary

        // move back outside is primary?
        jellyPulse.setParams(scene6pulseSpeed / 2.0, scene6pulseAmount * 2.5,
                             1);
        jellyEffectChain.process(jellyCreatureMesh, sceneTime);
        jellyCreatureMesh.update();
      }
    }
    if (!isPrimary()) {
      for (int i = 0; i < jellies.size(); ++i) {
        jellies[i].pos().set(state().jellyX[i], state().jellyY[i],
                             state().jellyZ[i]);
        jellies[i].quat().set(state().jellyQuatW[i], state().jellyQuatX[i],
                              state().jellyQuatY[i], state().jellyQuatZ[i]);
      }
    }
  }

  void drawScene6(al::Graphics &g) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    g.depthTesting(true);
    // g.clear(0.1, 0.0, 0.3 * (1.0 - (sceneTime- / (557.0))));
    g.clear(0.1, 0.0, 0.3 * (1.3 - (sceneTime / 557.0)));
    g.lighting(true);
    light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
    light.ambient(al::RGB(1.0, 1.0, 1.0));
    light.diffuse(al::RGB(1, 1, 1.0));
    g.light(light);
    material.specular(1.0);
    material.shininess(200);
    g.material(material);
    g.pointSize(pointSizeScene6.get());

    for (int i = 0; i < jellies.size(); ++i) {
      g.pushMatrix();
      g.translate(state().jellyX[i], state().jellyY[i], state().jellyZ[i]);
      g.rotate(al::Quatf(state().jellyQuatW[i], state().jellyQuatX[i],
                         state().jellyQuatY[i], state().jellyQuatZ[i]));
      g.pointSize(2.0);
      g.color(1.0f, 0.4f, 0.7f, state().flicker);
      g.draw(jellyCreatureMesh);
      g.popMatrix();
    }
  }

  void onSound(al::AudioIOData &io) override {
    if (isPrimary()) {
      spatializer->prepare(io);

      if (sceneIndex == 1) {
        mSequencer1.render(io);
      } else if (sceneIndex == 2) {
        mSequencer2.render(io);
      } else if (sceneIndex == 3) {
        mSequencer3.render(io);
      } else if (sceneIndex == 4) {
        mSequencer4.render(io);
      } else if (sceneIndex == 5) {
        mSequencer5.render(io);
      } else if (sceneIndex == 6) {
        mSequencer6.render(io);
      }

      // Joels idea to put sub routing before spatialization? this seems to
      // seg fault comment it out if it breaks things for (unsigned sample =
      // 0; sample < io.framesPerBuffer(); sample++) {
      //   io.out(47, sample) = io.out(0);
      // }
      // comment this version out of it breaks things
      while (io()) {
        io.out(47) = io.out(0); // io.bus(0) * 0.5;
      }

      spatializer->renderBuffer(io, {0, 0, 0}, io.outBuffer(0),
                                io.framesPerBuffer());
      spatializer->finalize(io);

      // while (io()) {
      //   io.out(47) = io.out(0); // io.bus(0) * 0.5;
      // }
    }
  }

  al::SynthSequencer mSequencer1;
  al::SynthSequencer mSequencer2;
  al::SynthSequencer mSequencer3;
  al::SynthSequencer mSequencer4;
  al::SynthSequencer mSequencer5;
  al::SynthSequencer mSequencer6;

  al::SynthSequencer &sequencer1() { return mSequencer1; }
  al::SynthSequencer &sequencer2() { return mSequencer2; }
  al::SynthSequencer &sequencer3() { return mSequencer3; }
  al::SynthSequencer &sequencer4() { return mSequencer4; }
  al::SynthSequencer &sequencer5() { return mSequencer5; }
  al::SynthSequencer &sequencer6() { return mSequencer6; }
};

int main() {
  MyApp app;

  if (al::sphere::isSphereMachine())
    app.configureAudio(44100, 512, 60, 0);
  else
    app.configureAudio(44100, 512, 2, 0);
  app.start();
  return 0;
}

std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}
