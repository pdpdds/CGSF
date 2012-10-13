/* -*- C++ -*- */
// $Id: URL_Visitor_Factory.cpp 91673 2010-09-08 18:49:47Z johnnyw $

#include "URL_Visitor_Factory.h"

URL_Visitor_Factory::~URL_Visitor_Factory (void)
{
}

URL_Visitor *
URL_Validation_Visitor_Factory::make_visitor (void)
{
  URL_Visitor *v;

  ACE_NEW_RETURN (v,
                  URL_Validation_Visitor,
                  0);

  return v;
}

Command_Processor *
URL_Validation_Visitor_Factory::make_command_processor (void)
{
  Command_Processor *cp;

  ACE_NEW_RETURN (cp,
                  Command_Processor,
                  0);
  return cp;
}

URL_Visitor *
URL_Download_Visitor_Factory::make_visitor (void)
{
  URL_Visitor *v;

  ACE_NEW_RETURN (v,
                  URL_Download_Visitor,
                  0);
  return v;
}

Command_Processor *
URL_Download_Visitor_Factory::make_command_processor (void)
{
  return 0;
}
