#pragma once

class Broadcaster
{
  private:
    static Broadcaster *instance;
    Broadcaster();

  public:
    static Broadcaster *getInstance();
};