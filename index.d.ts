export interface FontDescriptor {
  readonly path: string;
  readonly style: string;
  readonly width: number;
  readonly family: string;
  readonly weight: number;
  readonly italic: boolean;
  readonly monospace: boolean;
  readonly postscriptName: string;
}

/**
 * Fetches monospace fonts in the system
 *
 * @example
 * getMonospaceFontsSync();
 * @returns All monospace fonts descriptors available
 */
export function getMonospaceFontsSync(): FontDescriptor[];

/**
 * Returns trough a callback all monospace fonts descriptors available on the system
 *
 * @param callback Contains the font data
 * @example
 * getMonospaceFonts((fonts) => { ... });
 */
export function getMonospaceFonts(
  callback: (fonts: FontDescriptor[]) => void
): void;
