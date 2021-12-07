/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "db.h"

#include "metaDef.h"

#include "thash.h"

struct SMetaDB {
  // DB
  DB *      pStbDB;
  DB *      pNtbDB;
  SHashObj *pCtbMap;
  DB *      pSchemaDB;
  // IDX
  SHashObj *pIdxMap;
  DB *      pNameIdx;
  DB *      pUidIdx;
  // ENV
  DB_ENV *pEvn;
};

static SMetaDB *metaNewDB();
static void     metaFreeDB(SMetaDB *pDB);

int metaOpenDB(SMeta *pMeta) {
  int      ret;
  SMetaDB *pDB;

  pMeta->pDB = metaNewDB();

#if 0
  // create the env
  ret = db_env_create(&(pMeta->pDB->pEvn), 0);
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = pMeta->pDB->pEvn->open(pMeta->pDB->pEvn, pMeta->path, DB_CREATE | DB_INIT_MPOOL, 0);
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = db_create(&(pMeta->pDB->pDB), pMeta->pDB->pEvn, 0);
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = db_create(&(pMeta->pDB->pSchemaDB), pMeta->pDB->pEvn, 0);
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = db_create(&(pMeta->pDB->pIdx), pMeta->pDB->pEvn, 0);
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = pMeta->pDB->pDB->open(pMeta->pDB->pDB, /* DB structure pointer */
                              NULL,            /* Transaction pointer */
                              "meta.db",       /* On-disk file that holds the database */
                              NULL,            /* Optional logical database name */
                              DB_BTREE,        /* Database access method */
                              DB_CREATE,       /* Open flags */
                              0);              /* File mode */
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = pMeta->pDB->pSchemaDB->open(pMeta->pDB->pSchemaDB, /* DB structure pointer */
                                    NULL,                  /* Transaction pointer */
                                    "meta.db",             /* On-disk file that holds the database */
                                    NULL,                  /* Optional logical database name */
                                    DB_BTREE,              /* Database access method */
                                    DB_CREATE,             /* Open flags */
                                    0);                    /* File mode */
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = pMeta->pDB->pIdx->open(pMeta->pDB->pIdx, /* DB structure pointer */
                               NULL,             /* Transaction pointer */
                               "index.db",       /* On-disk file that holds the database */
                               NULL,             /* Optional logical database name */
                               DB_BTREE,         /* Database access method */
                               DB_CREATE,        /* Open flags */
                               0);               /* File mode */
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }

  ret = pMeta->pDB->pDB->associate(pMeta->pDB->pDB,  /* Primary database */
                                   NULL,             /* TXN id */
                                   pMeta->pDB->pIdx, /* Secondary database */
                                   metaIdxCallback,  /* Callback used for key creation */
                                   0);               /* Flags */
  if (ret != 0) {
    // TODO: handle error
    return -1;
  }
#endif

  return 0;
}

void metaCloseDB(SMeta *pMeta) {
  metaFreeDB(pMeta->pDB);
  pMeta->pDB = NULL;
#if 0
  if (pMeta->pDB) {
    if (pMeta->pDB->pIdx) {
      pMeta->pDB->pIdx->close(pMeta->pDB->pIdx, 0);
      pMeta->pDB->pIdx = NULL;
    }

    if (pMeta->pDB->pSchemaDB) {
      pMeta->pDB->pSchemaDB->close(pMeta->pDB->pSchemaDB, 0);
      pMeta->pDB->pSchemaDB = NULL;
    }

    if (pMeta->pDB->pDB) {
      pMeta->pDB->pDB->close(pMeta->pDB->pDB, 0);
      pMeta->pDB->pDB = NULL;
    }

    if (pMeta->pDB->pEvn) {
      pMeta->pDB->pEvn->close(pMeta->pDB->pEvn, 0);
      pMeta->pDB->pEvn = NULL;
    }

    free(pMeta->pDB);
  }
#endif
}

int metaSaveTableToDB(SMeta *pMeta, STbCfg *pTbCfg) {
#if 0
  tb_uid_t uid;
  DBT      key = {0};
  DBT      value = {0};
  char     buf[256];
  void *   pBuf;
  int      bsize;

  if (pTbCfg->type == META_SUPER_TABLE) {
    uid = pTbCfg->stbCfg.suid;
  } else {
    uid = metaGenerateUid(pMeta);
  }

  key.size = sizeof(uid);
  key.data = &uid;

  pBuf = buf;
  value.size = metaEncodeTbCfg(&pBuf, pTbCfg);
  value.data = buf;

  pMeta->pDB->pDB->put(pMeta->pDB->pDB, NULL, &key, &value, 0);
#endif

  return 0;
}

int metaRemoveTableFromDb(SMeta *pMeta, tb_uid_t uid) {
  // TODO
}

/* ------------------------ STATIC METHODS ------------------------ */
static SMetaDB *metaNewDB() {
  SMetaDB *pDB;
  pDB = (SMetaDB *)calloc(1, sizeof(*pDB));
  if (pDB == NULL) {
    return NULL;
  }

  pDB->pCtbMap = taosHashInit(0, MurmurHash3_32, false, HASH_NO_LOCK);
  if (pDB->pCtbMap == NULL) {
    metaFreeDB(pDB);
    return NULL;
  }

  pDB->pIdxMap = taosHashInit(0, MurmurHash3_32, false, HASH_NO_LOCK);
  if (pDB->pIdxMap == NULL) {
    metaFreeDB(pDB);
    return NULL;
  }

  return pDB;
}

static void metaFreeDB(SMetaDB *pDB) {
  if (pDB == NULL) {
    if (pDB->pIdxMap) {
      taosHashCleanup(pDB->pIdxMap);
    }

    if (pDB->pCtbMap) {
      taosHashCleanup(pDB->pCtbMap);
    }

    free(pDB);
  }
}