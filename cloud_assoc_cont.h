#ifndef CLOUD_ASSOC_CONT_H
#define CLOUD_ASSOC_CONT_H


/*! \brief - helper type for constant array
*/
typedef struct {

    const char *p;
    unsigned    size;
} t_array_content;


const t_array_content empty_content = { NULL, 0 };


/*! \brief - associative chained list of pointers
 * into const memory (do not care if it is linear or
 * circle buffer)
*/
template <unsigned N> class t_assoc_chlist_content {

public:
  struct t_juni_map {

    t_array_content   key;
    t_array_content   item;
  };

  t_juni_map v[N+1];  //+1 act as endmark and also as defat value for unknow get()
  t_assoc_chlist_content *succ;  //pointer to item which following

private:

  t_juni_map *access(const char *key){

      uint8_t i=0, n=strlen(key);

      while(NULL != v[i].key.p){

        if(v[i].key.p == key)  //1. const pointer equality - fastest
            break;

        if(v[i].key.size == n)  //2. full mem comparition - slow
          if(0 == memcmp(key, v[i].key.p, n))
            break;

        if(++i >= sizeof(v)/sizeof(v[0]))
          return NULL;
      }

      return &v[i];
  }

public:

  const t_array_content *set(const char *key, const t_array_content &v, bool mustexist = true){

      t_juni_map *i = access(key);
      if(NULL != i){

        if(NULL == i->key.p){

            if(mustexist)
                return NULL;

            i->key.p = key;
            i->key.size = strlen(key);
        }

        i->item.p = v.p;
        i->item.size = v.size;
        return &i->item;
      }

      return NULL;
  }

  const t_array_content *add(const char *key, const t_array_content &v){

      return set(key, v, false);
  }

  const t_array_content *get(const char *key, const t_array_content *def = &empty_content){

      t_juni_map *i = access(key);
      if(NULL != i->key.p)  //not key like entered
        return &i->item;

      return (def) ? def : NULL;
  }

  void reset(){

      for(unsigned i = 0; i < N; i++)
        v[i].item = empty_content;
  }

  t_assoc_chlist_content(const t_array_content _keys[]){

    for(unsigned i = 0; (NULL != _keys[i].p) && (i < N); i++){

        v[i].key.p = _keys[i].p;
        v[i].key.size = (_keys[i].size) ? _keys[i].size : strlen(_keys[i].p);
        v[i].item = empty_content;
    }

    succ = NULL;
  }

  virtual ~t_assoc_chlist_content(){;}
};


#endif // CLOUD_ASSOC_CONT_H
