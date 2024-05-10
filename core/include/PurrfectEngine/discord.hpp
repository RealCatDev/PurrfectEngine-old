#ifndef PURRENGINE_DISCORD_HPP_
#define PURRENGINE_DISCORD_HPP_

#include <discord_rpc.h>

namespace PurrfectEngine {
    class discord
    {  
    public:
        discord(const char *applicationId){
            DiscordEventHandlers handlers;
            memset(&handlers, 0, sizeof(handlers));
            Discord_Initialize(applicationId, &handlers, 1, NULL);
        }
        ~discord(){
            Discord_Shutdown();
        }

        void SetRichPresence(const char* details, const char* largeImageKey, const char* largeImageText, const char* smallImageKey, const char* smallImageText) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.details = details;
            discordPresence.startTimestamp = 0;
            discordPresence.largeImageKey = largeImageKey;
            discordPresence.largeImageText = largeImageText;
            discordPresence.smallImageKey = smallImageKey;
            discordPresence.smallImageText = smallImageText;
            discordPresence.instance = 0;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceState(const char* newState) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.state = newState;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceDetails(const char* newDetails) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.details = newDetails;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceLargeImage(const char* newImageKey) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.largeImageKey = newImageKey;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceLargeImageText(const char* newImageText) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.largeImageText = newImageText;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceSmallImage(const char* newImageKey) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.smallImageKey = newImageKey;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceSmallImageText(const char* newImageText) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.smallImageText = newImageText;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceStartTimestamp(int64_t startTime) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.startTimestamp = startTime;

            Discord_UpdatePresence(&discordPresence);
        }

        void UpdateRichPresenceEndTimestamp(int64_t endTime) {
            DiscordRichPresence discordPresence;
            memset(&discordPresence, 0, sizeof(discordPresence));
            discordPresence.endTimestamp = endTime;

            Discord_UpdatePresence(&discordPresence);
        }

        void ClearRichPresence() {
            Discord_ClearPresence();
        }


    };
    
}

#endif