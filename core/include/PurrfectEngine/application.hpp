#ifndef PURRENGINE_APPLICATION_HPP_
#define PURRENGINE_APPLICATION_HPP_

namespace PurrfectEngine {

  class application {
  public:
    static application *getInstance();
  public:
    application();
    ~application();

    void        setAssetDir(const char *);
    const char *getAssetDir() const;
  private:
    inline static application *sInstance = nullptr;
  private:
    char *mAssetDir;
  };

}

#endif