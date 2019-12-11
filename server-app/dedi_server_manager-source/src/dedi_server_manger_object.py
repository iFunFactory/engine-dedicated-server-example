#!/usr/bin/env python
# vim: fileencoding=utf-8 tabstop=2 softtabstop=2 shiftwidth=2 expandtab
# Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
#
# This work is confidential and proprietary to iFunFactory Inc. and
# must not be used, disclosed, copied, or distributed without the prior
# consent of iFunFactory Inc.


# Example.
#
# AppId/ProjectName
#   MyGame
#
# Object Model
#
#   {
#     "Character": {
#       "Name": "String KEY",
#       "Level": "Integer",
#       "Hp": "Integer",
#       "Mp": "Integer"
#     }
#   }
#
# Create/Fetch/Delete Example
#
#   import my_game_object
#
#   def Example():
#     # Initialize
#     # my_game_object.initialize(mysql id, mysql pw, mysql server address, database name, zookeeper hosts, app id)
#     my_game_object.initialize('db_id', 'db_pw', '127.0.0.1', 'my_game_db', '127.0.0.1', 'MyGame')
#
#     # Create an object
#     character = my_game_object.Character.create('mycharacter')
#     character.set_Level(1)
#     character.set_Hp(150)
#     character.set_Mp(70)
#     character.commit()
#
#     # Fetch an object
#     character = my_game_object.Character.fetch_by_Name('mycharacter2')
#     character.set_Level(character.get_Level() + 1)
#     print character.get_Name()
#     print character.get_Level()
#     print character.get_Hp()
#     print character.get_Mp()
#     character.commit()
#
#     # Delete an object
#     character = my_game_object.Character.fetch_by_Name('mycharacter3')
#     character.delete()
#     character.commit()


import binascii
import sys
sys.path.append('/usr/share/funapi/python')

import funapi.object.object as funapi


def initialize(user, password, host, database, zookeeper_hosts, app_name):
  funapi.MysqlConnection.initialize(user, password, host, database)
  funapi.Zookeeper.initialize(zookeeper_hosts, app_name)

