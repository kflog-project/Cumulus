/***********************************************************************
 **
 **   Frequency.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2018 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class Frequency
 *
 * \author Axel Pauli
 *
 * \brief Class to handle frequency and type of an airfield.
 *
 * This class contains a frequency and its related type.
 *
 * \date 2018
 *
 */

#ifndef Frequency_h
#define Frequency_h

#include <QString>


class Frequency
{
 public:

  Frequency( float frequency, QString type="" )
  {
    m_frequency = frequency;
    m_type = type;
  }

  virtual ~Frequency()
  {
  }

  void setFrequencyAndType( float frequency, QString type )
  {
    m_frequency = frequency;
    m_type = type;
  }

  QString& getType()
  {
    return m_type;
  }

  void setType(QString& type)
  {
    m_type = type;
  }

  float getFrequency() const
  {
    return m_frequency;
  }

  void setFrequency( const float frequency )
  {
    m_frequency = frequency;
  }

  /**
   * @return The frequency with type as string.
   */
  QString frequencyAsString( Frequency frequency ) const
    {
      QString fs;

      if( frequency.getFrequency() > 0 )
        {
          fs = QString("%1").arg(frequency.getFrequency(), 0, 'f', 3);

          QString& type = frequency.getType();

          if( type.size() > 0 )
            {
              fs += " (" + type + ")";
            }
        }

      return fs;
    };

  /**
   * @return The frequency with type as string.
   */
  QString frequencyAsString() const
  {
    return frequencyAsString( *this );
  }


 protected:

  float m_frequency;
  QString m_type;
};

#endif
