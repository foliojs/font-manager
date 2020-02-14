declare module 'font-manager' {
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

    export interface QueryFontDescriptor {
        readonly path?: string;
        readonly style?: string;
        readonly width?: number;
        readonly family?: string;
        readonly weight?: number;
        readonly italic?: boolean;
        readonly monospace?: boolean;
        readonly postscriptName?: string;
    }

    /**
     * Fetches fonts in the system
     * 
     * @example
     * getAvailableFontsSync();
     * @returns All fonts descriptors available
     */
    export function getAvailableFontsSync(): FontDescriptor[];

    /**
     * Returns trough a callback all fonts descriptors available on the system
     * 
     * @param callback Contains the font data
     * @example
     * getAvailableFonts((fonts) => { ... });
     */
    export function getAvailableFonts(callback: (fonts: FontDescriptor[]) => void): void;

    /**
     * Queries all the fonts in the system matching the given parameters
     *
     * @param fontDescriptor Query parameters
     * @example
     * findFontsSync({ family: 'Arial' });
     * findFontsSync();
     * @returns All fonts descriptors matching query parameters
     */
    export function findFontsSync(fontDescriptor: QueryFontDescriptor | undefined): FontDescriptor[];

    /**
     * Queries all the fonts in the system matching the given parameters
     * 
     * @param fontDescriptor Query parameters
     * @param callback Contains the font data
     * @example
     * findFonts({ family: 'Arial' }, (fonts) => { ... });
     * findFonts((fonts) => { ... });
     */
    export function findFonts(fontDescriptor: QueryFontDescriptor | undefined, callback: (fonts: FontDescriptor[]) => void): void;

    /**
     * Find only one font matching the given query. This function always returns
     * a result (never null), so sometimes the output will not exactly match the
     * input font descriptor if not all input parameters could be met
     * 
     * @param fontDescriptor Query parameters
     * @example
     * findFontSync({ family: 'Arial', weight: 700 });
     * findFontSync();
     * @returns Only one font description matching those query parameters
     */
    export function findFontSync(fontDescriptor: QueryFontDescriptor): FontDescriptor;

    /**
     * Find only one font matching the given query. This function always returns
     * a result (never null), so sometimes the output will not exactly match the
     * input font descriptor if not all input parameters could be met
     *
     * @param fontDescriptor Query parameters
     * @example
     * findFont({ family: 'Arial', weight: 700 }, (font) => { ... });
     * findFont((font) => { ... });
     * @returns Only one font description matching those query parameters
     */
    export function findFont(fontDescriptor: QueryFontDescriptor | undefined, callback: (font: FontDescriptor) => void): void;

    /**
     * Substitutes the font with the given post script name with another font
     * that contains the characters in text. If a font matching post script
     * name is not found, a fount containing the given characters is still
     * returned. If a font matching post script name is found, its
     * characteristics (bold, italic, etc) are used to find a suitable
     * replacement. If the font already contains the characters in text, it is
     * not replaced and the font descriptor for the original font is returned
     * 
     * @param postscriptName Name of the font to be replaced
     * @param text Characters for matching
     * @returns Only one font description matching the function description
     */
    export function substituteFontSync(postscriptName: string, text: string): FontDescriptor;

    /**
     * Substitutes the font with the given post script name with another font
     * that contains the characters in text. If a font matching post script
     * name is not found, a fount containing the given characters is still
     * returned. If a font matching post script name is found, its
     * characteristics (bold, italic, etc) are used to find a suitable
     * replacement. If the font already contains the characters in text, it is
     * not replaced and the font descriptor for the original font is returned
     *
     * @param postscriptName Name of the font to be replaced
     * @param text Characters for matching
     */
    export function substituteFont(postscriptName: string, text: string, callback: (font: FontDescriptor) => void): void;
}
