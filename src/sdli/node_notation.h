#ifndef SDLI_NODE_NOTATION_H
#define SDLI_NODE_NOTATION_H

#include <vuid.h>

//
// macros
//

/* Internal variable name for the state. Used by macros. */
#define NN_STATE_VAR node_notation_state

/* Create a new node. Adds a stack variable, VAR, to store the result. */
#define NN_BUILD_NEW(VAR)                       \
  VNode* VAR = NULL;                            \
  for (NNState NN_STATE_VAR = NN__BeginBuild(); \
       NN_STATE_VAR < NN_STATE_SUCCESS;         \
       NN_STATE_VAR = NN__EndBuild(NN_STATE_VAR, &VAR))

/* Append a new node to the parent NODE. */
#define NN_BUILD_APPEND(NODE)                        \
  for (NNState NN_STATE_VAR = NN__BeginAppend(NODE); \
       NN_STATE_VAR < NN_STATE_SUCCESS;              \
       NN_STATE_VAR = NN__EndAppend(NN_STATE_VAR))

/*
 * Get the current node notation state. Don't use this directly. State is an
 * implementation (specifically a consequence of C macro tricks) that should
 * be avoid in node trees. Use NM_CALL and NM_CALLABLE instead.
 */
#define NN_STATE() NN_STATE_VAR

/*
 * Mark a function as callable. It will be passed the node notation state of the
 * current NN_BUILD_NEW or NN_BUILD_APPED tree.
 */
#define NN_CALLABLE NNState NN_STATE_VAR

/* Call a function as if it was part of the current node notation tree. */
#define NN_CALL(FN, ...) FN(NN_STATE(), __VA_ARGS__)

/* Get the node currently under construction. */
#define NN_SELF() NN__Self()

/* Get the parent of the node currently under construction. */
#define NN_PARENT() NN__Parent()

/* Append a node to the current parent within a node notation tree. */
#define NN_NODE(TAG, ...)                                                      \
  for (NN_STATE_VAR =                                                          \
           NN__BeginNode(NN_STATE_VAR, TAG,                                    \
                         &((NodeAttributesWrap){.attrs = __VA_ARGS__}).attrs); \
       NN_STATE_VAR < NN_STATE_SUCCESS;                                        \
       NN_STATE_VAR = NN__EndNode(NN_STATE_VAR))

/* Append a box node to the current parent within a node notation tree. */
#define NN_BOX(...) NN_NODE(V_NODE_BOX, __VA_ARGS__)

/* Append a text node to the current parent within a node notation tree. */
#define NN_TEXT(...) NN_NODE(V_NODE_TEXT, __VA_ARGS__)

/* Append an image node to the current parent within a node notation tree. */
#define NN_IMAGE(...) NN_NODE(V_NODE_IMAGE, __VA_ARGS__)

//
// types
//

typedef struct NodeAttributes {
  const char* id;
  const char* sclass;
  VPopover popover;
  void* data;
  const char* src;
  const char* text;
  bool hidden;
  VNodeEventListener on_click;
  VNodeEventListener on_mouse_enter;
  VNodeEventListener on_mouse_leave;
} NodeAttributes;

typedef struct NodeAttributesWrap {
  NodeAttributes attrs;
} NodeAttributesWrap;

typedef enum NNState {
  NN_STATE_INIT = 0,
  NN_STATE_SUCCESS = 1,
  NN_STATE_ERROR = 2,
} NNState;

//
// public functions
//

/*
 * The Node Notation system is a way to build vuid node hierarchies in C code.
 *
 * Nodes can be built with "new", creating a new node tree or "append",
 * appending a new tree to an existing parent. Trees start with NN_BUILD_NEW or
 * NN_BUILD_APPEND. The system keeps track of the current node and its parent,
 * accessible with NN_SELF() and NN_PARENT(), respectively. Withing the tree,
 * nodes can be appended with NN_NODE(), NN_BOX(), NN_TEXT(), and NN_IMAGE().
 *
 * The system uses macros and trees can get quite complex. Subtrees can be put
 * into functions. The function needs to be marked with NN_CALLABLE and within
 * the tree, called with NN_CALL().
 *
 * NN_BUILD_NEW(new_node) {
 *  NN_BOX({.id = "new_node"}) {
 *    NN_BOX({.id = "button"}) {
 *      NN_TEXT({.text = "Click me"});
 *    }
 *  }
 * }
 *
 * The system uses a stack behind the scenes to track the current node and its
 * parent. The system also uses C macro magic to get everything to work. The
 * limitation is that code within a tree should not use break or return. The
 * node notation should be treated as a template, like handlebars, or as if it
 * was xml.
 */

/* Node Notation initialization functions. */

void NodeNotation_Init(void);
void NodeNotation_Drop(void);

/* Functions used by macros. Do not use directly. */

NNState NN__BeginNode(NNState token, VNodeTag tag, const NodeAttributes* attrs);
NNState NN__EndNode(NNState token);
NNState NN__BeginBuild(void);
NNState NN__EndBuild(NNState token, VNode** result);
NNState NN__BeginAppend(VNode* parent);
NNState NN__EndAppend(NNState token);
VNode* NN__Self(void);
VNode* NN__Parent(void);

#endif  // SDLI_NODE_NOTATION_H
