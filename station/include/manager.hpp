#pragma once

/// Interface for managers
///
/// Each manager works similarly to a microservice
class IManager
{
public:
  virtual void init () = 0;
  virtual void stop () = 0;
  virtual ~IManager (){};
};
