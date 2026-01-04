export type VNode = {
  type: string | ( ( props: any ) => VNode | VNode[] | string | null );
  props: Record<string, any>;
  children: Array<VNode | string | null | undefined>;
};

export function h(
  type: VNode['type'],
  props: Record<string, any> | null,
  ...children: Array<VNode | string | null | undefined>
): VNode {
  return { type, props: props || {}, children };
}

export const Fragment = ( props: { children?: any } ) =>
  Array.isArray( props.children ) ? props.children : [ props.children ];

export function render( node: any, parent: HTMLElement ) {
  parent.innerHTML = '';
  parent.appendChild( toDOM( node ) );
}

function toDOM( node: any ): Node {
  if ( node === null || node === undefined ) {
    return document.createTextNode( '' );
  }
  if ( typeof node === 'string' || typeof node === 'number' ) {
    return document.createTextNode( String( node ) );
  }

  if ( Array.isArray( node ) ) {
    const frag = document.createDocumentFragment();
    node.forEach( child => frag.appendChild( toDOM( child ) ) );

    return frag;
  }

  if ( typeof node.type === 'function' ) {
    return toDOM( node.type( { ...node.props, children: node.children } ) );
  }

  const el = document.createElement( node.type as string );
  const props = node.props || {};
  Object.entries( props ).forEach( ( [ key, value ] ) => {
    if ( key === 'className' ) {
      if ( typeof value === 'string' ) {
        el.setAttribute( 'class', value );
      }
    }
    else if ( key.startsWith( 'on' ) && typeof value === 'function' ) {
      ( el as any )[ key.toLowerCase() ] = value;
    }
    else if ( value !== false && value !== null ) {
      el.setAttribute( key, String( value ) );
    }
  } );
  if ( node.children ) {
    node.children.forEach( ( child: any ) => {
      el.appendChild( toDOM( child ) );
    } );
  }

  return el;
}

// Expose globals for JSX pragma usage when bundled.
if ( typeof globalThis !== 'undefined' ) {
  ( globalThis as any ).h = h;
  ( globalThis as any ).Fragment = Fragment;
}
