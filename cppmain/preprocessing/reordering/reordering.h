
#ifndef REORDERING_H
#define REORDERING_H

#include "../../configs/strings.h"
#include "../../configs/config.h"

using namespace std;

typedef std::pair<uintT, uintE> degree_nodeid_t;

vector<vid_t> *generateRandomMapping(vid_t, eid_t,
                                     const vector<vid_t> &,
                                     const vector<vid_t> &);
vector<vid_t> *generateHubSortDBGMapping(vid_t, eid_t,
                                         const vector<vid_t> &,
                                         const vector<vid_t> &);
vector<vid_t> *generateHubClusterDBGMapping(vid_t, eid_t,
                                            const vector<vid_t> &,
                                            const vector<vid_t> &);
vector<vid_t> *generateDBGMapping(vid_t, eid_t,
                                  const vector<vid_t> &,
                                  const vector<vid_t> &);
vector<vid_t> *generateSortMapping(vid_t, eid_t,
                                   const vector<vid_t> &,
                                   const vector<vid_t> &);
vector<vid_t> *generateHubClusterMapping(vid_t, eid_t,
                                         const vector<vid_t> &,
                                         const vector<vid_t> &);
vector<vid_t> *generateHubSortMapping(vid_t, eid_t,
                                      const vector<vid_t> &,
                                      const vector<vid_t> &);

int verify_mapping(vid_t, eid_t,
                   const vector<vid_t> &,
                   const vector<vid_t> &,
                   const vector<vid_t> *);
vector<vid_t> *generate_mapping(const vid_t nv, const eid_t ne,
                                const vector<vid_t> &in_degrees,
                                const vector<vid_t> &out_degrees);

#endif // REORDERING_H